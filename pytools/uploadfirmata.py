import os
import sys
import subprocess
import click


"""
Fast upload arcontrol task to the arduino board, comparing to the original arduino ide. 
Xinfeng Chen, 2021-1-21

$ pyinstaller uploadfirmata.py
"""


_board = None
_hex_mtime = None
_arc_board = None
_arduino_debug_p = None
BOARD_MAP = {'Uno': 'arduino:avr:uno', 'Mega': 'arduino:avr:mega:cpu=atmega2560', 'Nano': 'arduino:avr:nano:cpu=atmega328'}
BOARD_FILE = {'Uno': 'StandardFirmata.ino.standard.hex', 'Mega': 'StandardFirmata.ino.mega.hex', 'Nano':'StandardFirmata.ino.nano328.hex'}
BOARD_MAP_t = dict([(v,k) for (k,v) in BOARD_MAP.items()])


@click.command()
@click.option("--board", help="Select the board to compile for. Unnecessary.")
@click.option("--port", help="The port to upload.")
def hello(board, port):
    if board is None:
        board = _arc_board
    else:
        assert board in BOARD_MAP_t

    if port is None:
        print('Error: Input PORT please!', file=sys.stderr)
        sys.exit(1)

    filename = 'StandardFirmata/' + BOARD_FILE[BOARD_MAP_t[board]]

    yes = do_upload(filename=filename, board=board, port=port)
    if yes:
        print('Uploading to board.')
        print('Finished!')
    else:
        sys.exit(1)


def do_upload(filename, board, port):
    print(f'Task=FIRMATA, Board={BOARD_MAP_t[board]}, Port={port}.')
    file_hex = filename
    arduino_dir = os.path.dirname(_arduino_debug_p)
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

    try:
        child = subprocess.Popen(cmd)
        exitcode = child.wait(10)
    except subprocess.TimeoutExpired:
        print("The board may unmatch with settings (Designer>PROFILE), or even has been broken!", file=sys.stderr)
        exitcode = 1
    finally:
        child.kill()

    if exitcode == 0:
        yes = True
    else:
        print("Error: uploading failed", file=sys.stderr)
        yes = False
    return yes


def get_arduino_debug_board():
    arduino_debug_p = ''
    with open('profile.xml', 'r') as f:
        for line in f.readlines():
            if '<ARDUINO_DEBUG>' in line:
                arduino_debug_p = line.replace('<ARDUINO_DEBUG>', '').replace('</ARDUINO_DEBUG>', '').strip()
                break
    arduino_debug_p = arduino_debug_p.replace('/', '\\').replace('\\', os.sep)  # sep unify as '/' or '\'

    with open('profile.xml', 'r') as f:
        for line in f.readlines():
            if '<ARDUINO_BOARD>' in line:
                confboard = line.replace('<ARDUINO_BOARD>', '').replace('</ARDUINO_BOARD>', '').strip()
                break
        else:
            raise Exception('Nonsense task config file!')

    global _arduino_debug_p, _arc_board
    _arduino_debug_p, _arc_board = arduino_debug_p, BOARD_MAP[confboard]


def init_check():
    if not os.path.isfile('profile.xml'):
        print("Error: Run the ArControl Designer and click PROFILE menu to set ARDUINO_DEBUG!", file=sys.stderr)
        return False

    get_arduino_debug_board()
    if _arduino_debug_p == '' or not os.path.isfile(_arduino_debug_p):
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
