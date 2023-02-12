# ![logo1](https://raw.githubusercontent.com/wiki/chenxinfeng4/ArControl/images/logo1.jpg)
---
ArControl is a **Arduino** based digital signals control system. A special application for ArControl is to establish a **animal behavioral platform (as Skinner box)**, which control devices to deliver stimulation and monitor behavioral response. ArControl is also useful to generate Optogenetic TTL pulses.

![Figure1](https://raw.githubusercontent.com/wiki/chenxinfeng4/ArControl/images/Figure1.jpg)

The new board -- Board V3

<img src="https://github.com/chenxinfeng4/ArControl/raw/PCB/BoardV3/images/boardv3.jpeg" width="400"/>

![Figure2](https://raw.githubusercontent.com/wiki/chenxinfeng4/ArControl/images/Figure1_2.jpg)

## Task visulization
ArControl Designer can export the behavior task to the [State Machine Cat](https://github.com/sverweij/state-machine-cat) (smcat), which offers beautifual	visualization of the behavior task.
See [converter](https://github.com/chenxinfeng4/ArControl-convert2-smcat). Take Go/No-Go for an example.

<center><img src="https://github.com/chenxinfeng4/ArControl-convert2-smcat/blob/master/playground/Go_NoGo.svg" height=600></center>

## Data file convert
ArControl use raw txt file to store recording data. It's easy to convert that to other general platforms.
>ArControl -> Python (auto) \
 ArControl -> Matlab (auto) \
 ArControl -> [NWB](https://www.nwb.org/) format (see [converter](https://github.com/chenxinfeng4/ArControl-convert2-nwb))
## Features

- [x] Comprehensive – it combines **software and hardware**, **behavioral task design and experimental data collection**.
- [x] Inexpensive – neither dedicated nor expensive hardware is essential.
- [x] General purpose - it’s applicable to **multiple behavioral tasks**.
- [x] Easy to use – behavior task can be decomposed by the straightforward State Notation concept, and designed via a friendly GUI **without need to master script language**. You are free from knownledge about C++ / Arduino / Matlab coding.
- [x] Real-time performance – it has **high temporal resolution (<1ms)** and **free from the load of computer**.

## Requirements

- Windows 7/10
- Arduino IDE

## Boards Support

- Arduino UNO (recommend; 6 Inputs, 8 Outputs)
- Arduino Mega2560 (more RAM; 6 Inputs, 8 Outputs)
- Arduino Nano328 (small size; 6 Inputs, 8 Outputs)

## Guidance

I prepared a [wiki page](https://github.com/chenxinfeng4/ArControl/wiki). You can find tons of useful thing there.

+ [Brief View](https://github.com/chenxinfeng4/ArControl/wiki/Brief_View) - Find out what does ArControl look like, and what can ArControl handle with.

+ [Ready Demos](https://github.com/chenxinfeng4/ArControl/wiki/Example_Light3) - View how to operate ArControl.

+ [Installation Guide](https://github.com/chenxinfeng4/ArControl/wiki/Installation_Guide) - Follow it to integrate ArControl into your project.

+ [PCB Assemble](https://github.com/chenxinfeng4/ArControl/wiki/PCB_assemble) - Assembling a simple PCB is very useful for a hardware platform.

Or you can watch the video tutorial here (Chinese language only/ 中文):
+ [How to install] https://www.bilibili.com/video/BV1zz4y1U7AY
+ [An example]. https://www.bilibili.com/video/BV13v411v7TN
+ [Another example for optogenetics]. https://www.bilibili.com/video/BV1yX4y1G7Zs

## Contact

  Contact me on email chenxinfeng@pku.edu.cn . If you find an issure, open a ticked on it.

## Cite This

Academic researchers using ArControl should cite our paper in their works.

- **Chen, X.**, and Li, H. (2017). [ArControl: An Arduino-Based Comprehensive Behavioral Platform with Real-Time Performance.](https://www.frontiersin.org/articles/10.3389/fnbeh.2017.00244/full) Front. Behav. Neurosci. 11, 244. doi: 10.3389/fnbeh.2017.00244.
- 陈昕枫. (2018). *基于 Arduino 系统的动物行为自动控制平台的构建及其应用* (Master's thesis, 华中科技大学). *[中国知网检索](https://oversea.cnki.net/KCMS/detail/detail.aspx?dbcode=CMFD&dbname=CMFD201901&filename=1018784966.nh&v=1uz5%25mmd2F1PVD8TWyrXO0NeiWYUDvM4exSqMqQm867oAanYBfLHiwtWRH4cS1c2OSIE0)*

## Reference codes

- QFirmata: https://github.com/firmata/protocol
- SCPP_ASSERT from Vladimir Kushnir

## Downloads

- Source-code download: https://github.com/chenxinfeng4/ArControl
- Binary-release download: https://github.com/chenxinfeng4/ArControl/releases
- PCB drafts download: https://github.com/chenxinfeng4/ArControl/releases

### License

ArControl is released under the GNU LGPL v2.1 license.
