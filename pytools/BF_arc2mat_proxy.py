# pyinstaller BF_arc2mat_proxy.py --name BF_arc2mat --exclude-module BF_arc2mat --upx-exclude BF_arc2mat.py --add-data BF_arc2mat.py;src 
# This's a pyinstaller proxy of BF_arc2mat.py, so the src/BF_arc2mat.py can be upgrade.
import re
import mat4py as sio
import sys
import os

if __name__ == '__main__':
    path = os.path.abspath(os.path.dirname(sys.argv[0]))
    sys.path.append(os.path.join(path, 'src'))

from BF_arc2mat import convert_txt2mat

if __name__ == '__main__':
    filetxts = sys.argv[1:]
    for filetxt in filetxts:
        convert_txt2mat(filetxt)
