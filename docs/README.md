# IOTutility 一个轻量级的 IOT 基础操作库

[TOC]

Base utility for IOT devices, networking, controls etc...


**IOTutility 的目的是提供一个易用的，轻量级的，能够快速使用的 IOT 编程功能库。**

### 1.  为什么要写一个 IOT 编程库？
    对于 IOT 设备而言，本身是功能比较单一，代码量也是越少越好，那么使用原生的库编程看起来应该是最好的选择。但是我在很多项目开发的初期却发现
    我总是在做一些重复的编码，比如建立基础的 TCP 和 UDP 通信，设计简单的通信协议来让 IOT 设备和管理服务器之间传递数据等等。其实这些工作可以
    相对完整的放在一个模块中，让我们快速的开始实施自己的想法。
### 2.  IOTutility 具有什么功能？
    目前对于 IOT 设备最主要的还是网络通信，所以 IOTutility 的主要功能是搭建网络环境和实现基本的通信协议例如 MQTT， IEC61850 等基于 TCP 的
    应用层协议
### 3.  IOTutility 有什么特点
    IOTutility 是个人业余开发而来，目前仅限于功能正常，没有经过大规模测试，那么唯一的特点就是简单。


## IOTutility 实现了哪些功能

目前只实现了基础的 UDP 和 TCP 通信，多线程和基础信号量，以及部分 MQTT 协议。

## _MQTT protocol implement_ MQTT 实现

app 目录中包含了一个 MQTT client 的实现，要实现一次 MQTT 通信，只需要以下几个步骤：
### 1.  创建一个 MQTT_Session
```
MQTT_Session* Session = MQTT_SessionCreate("198.41.30.241", 1883);
```
以上是创建一个到 iot.eclipse.org 的 MQTT 通信

### 2.  连接到服务器

```
Session->Connect(Session);
```
直接调用 Connect 接口，连接到创建的服务器地址

### 3.  订阅感兴趣的主题

```
Session->Subscribe(Session, "wuhan/#");
```
这样会订阅所有传递到 iot.eclipse.org 服务器以 _wuhan/_ 开头的所有消息，如果要处理这些消息，需要调用 Fetch 接口：
```
Session->Fetch(Session, topic_and_message);
```

### 4.  可以通过 Publish 接口直接上传消息给服务器

```
Session->Publish(Session, "wuhan/test", "hello", 5);
```

## 如何使用
### 1.  下载代码
```
git clone https://github.com/ruiers/IOTutility.git
```
### 2.  编译代码库
```
cd IOTutility
make
```
### 3.  编译示例程序并执行
```
make mqtt_client.bin
./mqtt_client.bin 
```

### 4.  输出示例
```
android@ubuntumysqlserver:~/share/github/IOTutility$ ./mqtt_client.bin 
a
topic     10:wuhan/test
message    5:hello
a
topic     10:wuhan/test
message    5:hello
z
```
按下 ‘a’ 发送一个消息到服务器，按下 ‘z’ 退出程序。 消息发送到 iot.eclipse.org， 我们的接收线程接收后会打印出消息的
主题和消息信息。