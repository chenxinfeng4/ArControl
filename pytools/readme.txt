#Setup this, before compile ArControl Designer/Recorder source code.

#How to set up this environment
## 1. install python package
pip install shutil pyinstaller mat4py
python == 3.7.0

## 2. Scripts to EXEs
pyinstaller BF_arc2mat_proxy.py
pyinstaller uploadtask.py
pyinstaller uploadfirmata.py

## 3. merge the 2 EXEs
Merge the content [dist/uploadfirmat/*], [dist/uploadtask/*] and [dist/BF_arcmat/*] into the new folder [py_tools/].
And then, place folder [py_tools/] aside with [task/] folder of ArControl.
