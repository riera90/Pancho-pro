import paho.mqtt.client as mqtt
import os, time
from ..main import stop_alarm, snooze_alarm, get_event, get_transport

# Define event callbacks
def on_connect(client, userdata, flags, rc):
    print('connected')
    print("rc: " + str(rc))
    pass

def on_message(client, obj, msg):
    print(msg.topic + " " + str(msg.qos) + " " + str(msg.payload))
    request = msg.payload.decode("utf-8")
    if request == 'hold':
        stop_alarm()
    if request == 'press':
        snooze_alarm()

def on_publish(client, obj, mid):
    print("publish: " + str(mid))
    pass

def on_subscribe(client, obj, mid, granted_qos):
    print("Subscribed: " + str(mid) + " " + str(granted_qos))

def on_log(client, obj, level, string):
    print(string)
    pass

def on_disconnect(client, userdata, rc):
    if rc == 0:
        print('disconnected')
        return

    print("Unexpected disconnection.")
    time.sleep(5)
    print('reconnecting')
    mqtt_init()

def mqtt_init():
    global mqtt_client
    mqtt_client = mqtt.Client(client_id=os.getenv('NODE_NAME'),
                              clean_session=True,
                              userdata=None,
                              transport="tcp")
    mqtt_client.username_pw_set(str(os.getenv('MQTT_USER')), str(os.getenv('MQTT_PASSWORD')))
    mqtt_client.connect(str(os.getenv('MQTT_BROKER')), int(os.getenv('MQTT_PORT')))
    mqtt_client.subscribe(str(os.getenv('MQTT_TOPIC_BED_BUTTON')), int(os.getenv('MQTT_QOS')))

    # Assign event callbacks
    mqtt_client.on_message = on_message
    mqtt_client.on_connect = on_connect
    mqtt_client.on_publish = on_publish
    mqtt_client.on_subscribe = on_subscribe
    mqtt_client.on_disconnect = on_disconnect

def mqtt_disconnect():
    global mqtt_client
    mqtt_client.disconnect()

def loop():
    global mqtt_client
    mqtt_client.loop(timeout=1.0, max_packets=1)

def send_message(topic, payload):
    global mqtt_client
    mqtt_client.publish(topic=topic, payload=payload, qos=int(os.getenv('MQTT_QOS')), retain=False, properties=None)



