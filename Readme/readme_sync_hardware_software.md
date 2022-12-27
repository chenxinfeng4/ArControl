# ArControl Guidence - Synchronize API (2022-12-27)

![logo1](https://raw.githubusercontent.com/wiki/chenxinfeng4/ArControl/images/logo1.jpg)

+ Author: Xinfeng Chen (陈昕枫)
+ Email  : chenxinfeng@hust.edu.cn
+ Copyright (C) 2022, Huazhong University of Science and Technology.  GNU LGPL v2.1.
+ Source-code download: https://github.com/chenxinfeng4/ArControl
+ Binary-release download: https://github.com/chenxinfeng4/ArControl/releases
+ PCB drafts download:  https://github.com/chenxinfeng4/ArControl/releases
+ **Documents (Wiki)**: https://github.com/chenxinfeng4/ArControl/wiki


Cite This

+ **Chen, X.**, and Li, H. (2017). [ArControl: An Arduino-Based Comprehensive Behavioral Platform with Real-Time Performance.](https://www.frontiersin.org/articles/10.3389/fnbeh.2017.00244/full) Front. Behav. Neurosci. 11, 244. doi: 10.3389/fnbeh.2017.00244.


Reference Codes

+ QFirmata: https://github.com/firmata/protocol
+ SCPP_ASSERT from Vladimir Kushnir



## How to synchronize with other DAQ by hardware trigger?
A task is commonly triggered by `ArControl Recorder > START` button. However, the task can triggered from other style.  Open `ArControl Designer > File > Profile > "Start" Model`. There are `immediately`, `Trigger by Hardware (pin_11_HIGH)`, `Trigger by: Software (serial communicate)`, `Trigger by: Software or Hardware`.

## How to synchronize by other software trigger / python api / socket?
Use python `socket` to start/stop ArControl Recorder as like click the `"START"` button.
1. Open the `ArControl Recorder > Menu > Tool > Socket`.
2. Record the `ArControl RecorderMenu > Tool > Socket > PORT`, etc 20171.
3. Run the code `python ARCONTROL_DIR/pytools/testsocket.py`.
```python
# testsocket.py
import socket
import time
# %% create connection
tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
serve_ip = 'localhost'
serve_port = 20171       #default ArControl Recorder Socket PORT
tcp_socket.connect((serve_ip, serve_port))


def send_read(send_data):
    send_data_byte = send_data.encode("utf-8")
    tcp_socket.send(send_data_byte)

    from_server_msg = tcp_socket.recv(1024)
    print(from_server_msg.decode("utf-8"))

# %% Supported commands
cmds = ['query_record', 'start_record', 'stop_record']

for send_data in cmds:
    send_read(send_data)
    time.sleep(5)

```