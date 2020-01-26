import paho.mqtt.client as mqtt

def mqtt_init():
    mqtt_client = mqtt.Client('alarm clock')
    mqtt_client.username_pw_set('diego', 'password')
    mqtt_client.connect('riera90.com', 7707)
    mqtt_client.subscribe('/test', 0)

def get_mqtt():
    return 'mqtt'