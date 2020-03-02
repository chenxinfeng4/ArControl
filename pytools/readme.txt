#Setup this, before compile ArControl Designer/Recorder source code.

#How to set up this environment
## 1. install python package
pip install shutil, pyinstaller
python >= 3.7

## 2. Scripts to EXEs
pyinstaller uploadtask.py
pyinstaller uploadfirmata.py

## 3. merge the 2 EXEs
Merge the content [dist/uploadfirmat/*] and [dist/uploadtask/*] to new folder [py_tools/].
And then, place folder [py_tools/] aside with [task/] folder of ArControl.
