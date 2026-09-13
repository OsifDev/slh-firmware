import paho.mqtt.client as mqtt
import time, sys

DEV = "ESP_14335C6C32C0"
BASE = "slh/esp/" + DEV
CMD = sys.argv[1] if len(sys.argv) > 1 else "ping"
got = []

def on_connect(c, u, f, rc):
    print("connected rc=" + str(rc))
    c.subscribe(BASE + "/response")
    print("subscribed " + BASE + "/response")

def on_message(c, u, msg):
    got.append(msg.payload.decode())
    print("REPLY: " + msg.payload.decode())

c = mqtt.Client()
c.on_connect = on_connect
c.on_message = on_message
c.connect("broker.hivemq.com", 1883, 60)
c.loop_start()
time.sleep(2)
print("publishing: " + CMD)
c.publish(BASE + "/command", CMD)
time.sleep(6)
c.loop_stop()
print("done. replies: " + str(len(got)))
