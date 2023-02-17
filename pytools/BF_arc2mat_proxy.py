# pyinstaller BF_arc2mat_proxy.py --name BF_arc2mat --upx-exclude BF_arc2mat.py --add-data BF_arc2mat.py;src 
# This's a pyinstaller proxy of BF_arc2mat.py, so the src/BF_arc2mat.py can be upgrade.
import re
import mat4py as sio
import sys

sys.path.append('./src')

from BF_arc2mat import convert_txt2mat

if __name__ == '__main__':
    filetxts = sys.argv[1:]
    for filetxt in filetxts:
        convert_txt2mat(filetxt)
