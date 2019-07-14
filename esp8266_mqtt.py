# SubscriberTest.py
import paho.mqtt.client as mqtt
import time

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    client.subscribe("Home/esp32_sub")
    client.subscribe("Home/esp32_pub")

def on_message(client, userdata, msg):
    #print(msg.topic+" "+str(msg.payload))
    tt = time.localtime(time.time())
    print(str(tt.tm_hour)+":"+str(tt.tm_min)+" "+msg.topic+" "+str(msg.payload))

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect("xxx.xxx.xxx.xxx", 1883, 60)
client.loop_forever()
