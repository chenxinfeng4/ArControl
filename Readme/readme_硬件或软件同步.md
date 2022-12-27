# ArControl Guidence - Synchronize API (2022-12-27)

![logo1](https://raw.githubusercontent.com/wiki/chenxinfeng4/ArControl/images/logo1.jpg)

+ 作者: 陈昕枫, 北京脑科学与类脑研究中心
+ 邮箱  : chenxinfeng@hust.edu.cn
+ Copyright (C) 2022,  GNU LGPL v2.1.
+ 源代码下载: https://github.com/chenxinfeng4/ArControl
+ 可执行文件下载: https://github.com/chenxinfeng4/ArControl/releases
+ PCB草图下载:  https://github.com/chenxinfeng4/ArControl/releases
+ **说明文档 (Wiki)**: https://github.com/chenxinfeng4/ArControl/wiki

引用该工作

- **Chen, X.**, and Li, H. (2017). [ArControl: An Arduino-Based Comprehensive Behavioral Platform with Real-Time Performance.](https://www.frontiersin.org/articles/10.3389/fnbeh.2017.00244/full) Front. Behav. Neurosci. 11, 244. doi: 10.3389/fnbeh.2017.00244.

涉及到的三方源代码

- QFirmata: https://github.com/firmata/protocol
- SCPP_ASSERT from Vladimir Kushnir



## 如何使用 [硬件触发信号] 开启/同步ArControl记录?
任务的开始执行往往是通过 `ArControl 记录器> 开始` 按钮。但是也有其它的开始方式。打开 `ArControl 设计师> 文件> 选项> "开始运行"方式`. 你可以选择 `马上开始`, `触发由: 硬件（引脚D11 高电平）`, `触发由: 软件（串口通讯）` -- 默认, `触发由: 硬件或软件 ` 。

## 如何通过 [软件/python/socket] 后台开启/同步 ArControl记录?
可以使用 python  `socket` 模块来开启 ArControl Recorder，通过模拟点击"开始"按钮。

1. 打开 `ArControl Recorder > Menu > Tool > Socket`.
2. 记录下 `ArControl RecorderMenu > Tool > Socket > PORT`, 默认是 20171.
3. 运行下面的代码 `python ARCONTROL_DIR/pytools/testsocket.py`.
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