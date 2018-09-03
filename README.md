# IOTutility
Base utility for IOT devices, networking, controls etc...

## MQTT protocol implement

A simple MQTT clinet, which could publish messages to MQTT brokers, the default test code publich messages to iot.eclipse.org

1.  Install MQTT clients and run tht MQTT version 3.1 client for subscribing
~~~
mosquitto_sub -h iot.eclipse.org -t "wuhan/#" -v
~~~
2.  Run the default test 
~~~
 ./thread_test.bin
~~~

3. You could recvied the messages from iot.eclipse.org
~~~
wuhan/test 12
~~~
4. Screen shots

![pcap](https://raw.githubusercontent.com/ruiers/IOTutility/master/pcap_thread_test_iot.PNG)
![console](https://raw.githubusercontent.com/ruiers/IOTutility/master/pcap_thread_test_iot_responds.PNG)
