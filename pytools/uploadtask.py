import os
import sys
import subprocess
from tempfile import TemporaryDirectory
import click
import shutil
import time


"""
Fast upload arcontrol task to the arduino board, comparing to the original arduino ide. 
Xinfeng Chen, 2020-2-14
$ conda create -n py37 python=3.7.0
$ conda activate py37
$ pip install pyinstaller click
$ pyinstaller uploadtask.py
"""

"""
FOR DEBUG:
1. Compile to HEX file
> f'"{arduino_debug_p}" --board {_board} --verify --pref build.path={temp_path} {filename}'

--UNO--
> "D:\L_Arduino\arduino_debug.exe" --board arduino:avr:uno --verify --pref build.path=D:\test1 "F:\ArControl_github_sourcecode\arcRecorder\task\Task_1\Task_1.ino"
--MEGA--
> "D:\L_Arduino\arduino_debug.exe" --board arduino:avr:mega:cpu=atmega2560 --verify --pref build.path=D:\test1 "F:\ArControl_github_sourcecode\arcRecorder\task\Task_1\Task_1.ino"
--NANO--
> "D:\L_Arduino\arduino_debug.exe" --board arduino:avr:nano:cpu=atmega328 --verify --pref build.path=D:\test1 "F:\ArControl_github_sourcecode\arcRecorder\task\Task_1\Task_1.ino"

2. Upload HEX file to the board
> f'"{arduino_dir}/hardware/tools/avr/bin/avrdude" -q -q -C "{arduino_dir}/hardware/tools/avr/etc/avrdude.conf" -V -patmega328p -carduino -P{port} -b115200 -D -Uflash:w:"{file_hex}":i'

--UNO--
> "D:\L_Arduino\hardware/tools/avr/bin/avrdude" -q -q -C "D:\L_Arduino/hardware/tools/avr/etc/avrdude.conf" -V -patmega328p -carduino -PCOM13 -b115200 -D -Uflash:w:"D:\test1\Task_1.ino.hex":i
--MEGA--
> "D:\L_Arduino\hardware/tools/avr/bin/avrdude" -q -q -C "D:\L_Arduino/hardware/tools/avr/etc/avrdude.conf" -V -patmega2560 -cwiring -PCOM13 -b115200 -D -Uflash:w:"D:\test1\Task_1.ino.hex":i
--NANO--
> "D:\L_Arduino\hardware/tools/avr/bin/avrdude" -q -q -C "D:\L_Arduino/hardware/tools/avr/etc/avrdude.conf" -V -patmega328p -carduino -PCOM4 -b57600 -D -Uflash:w:"D:\test1\Task_1.ino.hex":i
"""

_board = None
_hex_mtime = None
_arc_board = None
BOARD_MAP = {'Uno': 'arduino:avr:uno', 'Mega': 'arduino:avr:mega:cpu=atmega2560', 'Nano': 'arduino:avr:nano:cpu=atmega328'}


@click.command()
@click.option("--arcupdate", is_flag=True, help="Build all ArControl sketchs.")
@click.option("--verify", is_flag=True, help="Build the sketch.")
@click.option("--upload", is_flag=True, help="Build and upload the sketch.")
@click.option("--board", default="arduino:avr:uno", help="Select the board to compile for. Unnecessary.")
@click.option("--port", help="The port to upload.")
@click.argument('filename', nargs=-1)
def hello(arcupdate, verify, upload, board, port, filename):
    """Simple program that greets NAME for a total of COUNT times."""
    ## CASE 1: --arcupdate
    if not (verify or upload) or arcupdate:
        do_arcupdate()
        return 0

    ## CASE 2: --verify or --upload
    if board == "arduino:avr:mega":
        board = "arduino:avr:mega:cpu=atmega2560"
    elif board == "arduino:avr:nano":
        board = BOARD_MAP['Nano']

    if len(filename)==0:
        print('No INO file input!', file=sys.stderr)
        sys.exit(1)
    elif len(filename)==1:
        filename = filename[0]
    else:
        print('Warning: the ino folder should not include SPACE!', file=sys.stderr)
        sys.exit(1)

    if verify or upload:
        print_header(filename)
        yes = do_verify(filename=filename, board=board)
        if yes and verify:
            print('Finished!')
        if not yes:
            sys.exit(1)
        
    ## CASE 3: --upload
    if upload:
        yes = do_upload(filename=filename, board=_board, port=port)
        if yes:
            print('Uploading to board.')
            print('Finished!')
        else:
            sys.exit(1)


def do_arcupdate():
    task_path = os.getcwd() + os.sep + 'task' + os.sep
    task_folders = [task_path + dir for dir in os.listdir(task_path)
                    if task_path + dir]
    for task_folder in task_folders:
        filenakename = os.path.basename(task_folder)
        filename = task_folder + os.sep + filenakename + '.ino'
        print_header(filename)
        yes = do_verify(filename)
        if yes:
            print('Succeed!\n\n')


def do_verify(filename, board=None):
    correctify_board(filename, board)

    if get_recompile(filename):
        # do compile
        yes = compile(filename)  # True: succeed; False: failed;
    else:
        # skip compile
        print('Already compiled at [%s]' % time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(_hex_mtime)))
        print(f"Skip compiling!", file=sys.stderr)
        yes = True

    return yes


def do_upload(filename, board, port):
    file_hex, file_hex_loader = get_hexname(filename)
    arduino_debug_p = get_arduino_debug()
    arduino_dir = os.path.dirname(arduino_debug_p)

    if board == BOARD_MAP['Uno']:
        cmd = f'"{arduino_dir}/hardware/tools/avr/bin/avrdude" -q -q -C "{arduino_dir}/hardware/tools/avr/etc/avrdude.conf" -V -patmega328p -carduino -P{port} -b115200 -D -Uflash:w:"{file_hex}":i'
    elif board == BOARD_MAP['Mega']:
        cmd = f'"{arduino_dir}/hardware/tools/avr/bin/avrdude" -q -q -C "{arduino_dir}/hardware/tools/avr/etc/avrdude.conf" -V -patmega2560 -cwiring -P{port} -b115200 -D -Uflash:w:"{file_hex}":i'
    elif board == BOARD_MAP['Nano']:
        cmd = f'"{arduino_dir}/hardware/tools/avr/bin/avrdude" -q -q -C "{arduino_dir}/hardware/tools/avr/etc/avrdude.conf" -V -patmega328p -carduino -P{port} -b57600 -D -Uflash:w:"{file_hex}":i'
    else:
        print("Error: ArControl did NOT support for {board}", file=sys.stderr)
        yes = False
        return yes

    cmd = cmd.replace(r'/', '\\').replace('\\', os.sep)
    print(cmd)

    try:
        child = subprocess.Popen(cmd)
        exitcode = child.wait(10)
    except subprocess.TimeoutExpired:
        print("Choose wrong board or the board has been broken!", file=sys.stderr)
        exitcode = 1
    finally:
        child.kill()

    if exitcode == 0:
        yes = True
    else:
        print("Error: uploading failed", file=sys.stderr)
        yes = False
    return yes


def print_header(filename):
    filenakename = os.path.basename(filename).replace('.ino', '')
    print(f"-----<{filenakename}>-----")


def correctify_board(filename, default_board):
    aconf_name = filename.replace('.ino', '.aconf')  # arcontrol task config file

    if not os.path.exists(aconf_name):
        return default_board  # arcontrol task config file NOT exist

    with open(aconf_name, 'r') as f:
        for line in f.readlines():
            if '<ARDUINO_BOARD>' in line:
                aconfboard = line.replace('<ARDUINO_BOARD>', '').replace('</ARDUINO_BOARD>', '').strip()
                break
        else:
            raise Exception('Nonsense task config file!')

    assert aconfboard in BOARD_MAP
    global _arc_board, _board
    _arc_board, _board = aconfboard, BOARD_MAP[aconfboard]
    return _board


def get_arduino_debug():
    arduino_debug_p = ''
    with open('profile.xml', 'r') as f:
        for line in f.readlines():
            if '<ARDUINO_DEBUG>' in line:
                arduino_debug_p = line.replace('<ARDUINO_DEBUG>', '').replace('</ARDUINO_DEBUG>', '').strip()
                break
    arduino_debug_p = arduino_debug_p.replace('/', '\\').replace('\\', os.sep)  # sep unify as '/' or '\'
    return arduino_debug_p


def get_ino_library_mtime():
    ino_library = os.getcwd() + os.sep + 'ino'
    file_in_ino = [ino_library + os.sep + file for file in os.listdir(ino_library)]
    file_in_ino = filter(os.path.isfile, file_in_ino)
    file_in_ino_date = [os.stat(file).st_mtime for file in file_in_ino]
    mtime = max(file_in_ino_date)
    return mtime


def compile(filename):
    with TemporaryDirectory() as temp_path:
        arduino_debug_p = get_arduino_debug()
        filenakename = os.path.basename(filename).replace('.ino', '')
        ## print HEADER
        print(f"Compile <{filenakename}>...")
        ## compile
        a = f'"{arduino_debug_p}" --board {_board} --verify --pref build.path={temp_path} {filename}'
        print(a)
        exitcode = os.system(a)
        ## print TAIL
        if exitcode != 0:
            # 烧录错误 #
            print('\nError! Compile failed.', file=sys.stderr)
            return False

        print('\nSucceed!')
        ## copy HEX file
        src = f"{temp_path}{os.sep}{filenakename}"
        file_hex, file_hex_loader = get_hexname(filename)
        shutil.move(f"{src}.ino.hex", file_hex)
        shutil.move(f"{src}.ino.with_bootloader.hex", file_hex_loader)
        return True


def get_hexname(filename):
    assert filename.endswith('.ino')
    file_hex = filename + '.standard.hex'
    file_hex_loader = filename + '.with_bootloader.standard.hex'
    return file_hex, file_hex_loader


def get_recompile(filename):
    file_hex, file_hex_loader = get_hexname(filename)
    if not (os.path.exists(file_hex) and os.path.exists(file_hex_loader)):
        return True

    ino_mtime = os.stat(filename).st_mtime  # 本task最后修改的时间
    library_mtime = get_ino_library_mtime()  # task 库最后修改的时间
    hex_mtime = [os.stat(file).st_mtime for file in [file_hex, file_hex_loader]]  # 最后编译的时间
    global _hex_mtime
    _hex_mtime = min(hex_mtime)
    is_hex_odd = max([ino_mtime, library_mtime]) > _hex_mtime  # 最近是否修改了源码。True: 重新编译HEX
    return is_hex_odd


def init_check():
    if not os.path.isfile('profile.xml'):
        print("Error: Run the ArControl Designer and click PROFILE menu to set ARDUINO_DEBUG!", file=sys.stderr)
        return False

    arduino_debug_p = get_arduino_debug()
    if arduino_debug_p == '' or not os.path.isfile(arduino_debug_p):
        print("Error: Run the ArControl Designer and click PROFILE menu to set ARDUINO_DEBUG!", file=sys.stderr)
        return False

    return True


if __name__ == '__main__':
    if os.path.isdir('task'):
        pass
    elif os.path.isdir('../task'):
        os.chdir('..')
    else:
        print("Error: the directory is not in ArControl", file=sys.stderr)
        sys.exit(1)

    if not init_check():
        sys.exit(1)

    hello()
