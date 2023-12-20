#!/usr/bin/env python3
# python3 /home/chenxinfeng/Documents/ArControl/pytools/uploadtask_cli.py --upload --board arduino:avr:uno --port /dev/ttyACM0 /home/chenxinfeng/Documents/ArControl/linux-amd64/task/Go_NoGo/Go_NoGo.ino
import os
import sys
import subprocess
from tempfile import TemporaryDirectory
import shutil
import time
import argparse

"""
Fast upload arcontrol task to the arduino board, comparing to the original arduino ide. 
Xinfeng Chen, 2023-4-24
$ conda create -n py37 python=3.7.0
$ conda activate py37
$ pip install pyinstaller click
$ pyinstaller uploadtask_cli.py
$ cp dist/uploadtask_cli/uploadtask_cli DEST/
"""

"""
It's for linux
"""

_board = None
_hex_mtime = None
_arc_board = None
arduino_cli = None
BOARD_MAP = {'Uno': 'arduino:avr:uno', 'Mega': 'arduino:avr:mega:cpu=atmega2560', 'Nano': 'arduino:avr:nano:cpu=atmega328'}

def main():
    parser = argparse.ArgumentParser(description='Arduino Sketch Builder')
    parser.add_argument('--arcupdate', action='store_true', help='Build all ArControl sketches.')
    parser.add_argument('--verify', action='store_true', help='Build the sketch.')
    parser.add_argument('--upload', action='store_true', help='Build and upload the sketch.')
    parser.add_argument('--board', type=str, default='arduino:avr:uno', help='Select the board to compile for. Unnecessary.')
    parser.add_argument('--port', type=str, help='The port to upload.')
    parser.add_argument('filename', nargs='+', help='The name of the file to build or upload.')
    
    args = parser.parse_args()    
    verify, upload, arcupdate = args.verify, args.upload, args.arcupdate
    board, port, filename = args.board, args.port, args.filename
    if sys.platform == 'linux':
        port = '/dev/' + port

    ## CASE 1: --arcupdate
    if not (verify or upload) or arcupdate:
        pass
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
        yes = True
        if yes and verify:
            print('Finished!')
        if not yes:
            sys.exit(1)
        
    ## CASE 3: --upload
    if upload:
        print('Uploading to board...')
        yes = do_compile_upload(filename=filename, board=board, port=port)
        if yes:
            print('Upload success!')
        else:
            sys.exit(1)


def do_compile_upload(filename, board, port):
    filedir = os.path.dirname(filename)
    if board not in BOARD_MAP.values(): return False

    cmd = f'"{arduino_cli}" compile -b {board} -p {port} -u "{filedir}"'
    cmd = cmd.replace(r'/', '\\').replace('\\', os.sep)
    cmd_l = [arduino_cli, 'compile', '-b', board, '-p', port, '-u', filedir]

    try:
        child = subprocess.Popen(cmd_l)
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



def init_check():
    global arduino_cli
    if os.path.isfile('arduino-cli') and os.access('arduino-cli', os.X_OK):
        arduino_cli = os.path.abspath('arduino-cli')
    elif shutil.which('arduino-cli'):
        arduino_cli = shutil.which('arduino-cli')
    else:
        print("Error: Can't find arduino-cli")
        return False
    return True


if __name__ == '__main__':
    # if os.path.isdir('task'):
    #     pass
    # elif os.path.isdir('../task'):
    #     os.chdir('..')
    # else:
    #     print("Error: the directory is not in ArControl", file=sys.stderr)
    #     sys.exit(1)

    if not init_check():
        sys.exit(1)

    main()
