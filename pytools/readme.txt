#Setup this, before compile ArControl Designer/Recorder source code.

#How to set up this environment
## 1. install python package
conda create -n arcontrol python=3.7.0
conda activate arcontrol
pip install pyinstaller mat4py graphviz


## 2. Scripts to EXEs
pyinstaller BF_arc2mat_proxy.py
pyinstaller uploadtask.py
pyinstaller uploadfirmata.py

## 3. merge the 2 EXEs
Merge the content [dist/uploadfirmat/*], [dist/uploadtask/*] and [dist/BF_arcmat/*] into the new folder [py_tools/].
And then, place folder [py_tools/] aside with [task/] folder of ArControl.
