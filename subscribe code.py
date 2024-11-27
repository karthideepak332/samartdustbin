import paho.mqtt.client as mqtt

# MQTT broker settings
MQTT_BROKER = "broker.hivemq.com"  # Replace with the broker address if different
MQTT_PORT = 1883                   # MQTT port
MQTT_TOPIC = "kec/sensors/data"    # Topic to subscribe to

# Callback for when the client receives a CONNACK response from the broker
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT broker!")
        client.subscribe(MQTT_TOPIC)
        print(f"Subscribed to topic: {MQTT_TOPIC}")
    else:
        print(f"Failed to connect, return code {rc}")

# Callback for when a PUBLISH message is received from the broker
def on_message(client, userdata, msg):
    print(f"Received message from topic {msg.topic}: {msg.payload.decode('utf-8')}")

# Initialize MQTT client
client = mqtt.Client("KEC_Subscriber")  # Unique client ID
client.on_connect = on_connect
client.on_message = on_message

# Connect to MQTT broker
print("Connecting to MQTT broker...")
client.connect(MQTT_BROKER, MQTT_PORT, 60)

# Start the loop to process callbacks and wait for messages
client.loop_forever()
