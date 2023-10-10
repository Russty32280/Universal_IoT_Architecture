#This code connects to a private shiftr.io instance
#using python and subscribes to a topic called chatting
#and publishes any keyboard input that is recieved.
#Any other messages published to this topic will also
#be printed to the console.

import paho.mqtt.client as mqtt

# Your shiftr.io MQTT broker information
shiftr_io_host = "rusmartlabclinic.cloud.shiftr.io"
shiftr_io_port = 1883  # Default MQTT port

# Your shiftr.io credentials
username = "rusmartlabclinic"
password = "ESP32Connect"
client_id = "Troy Laptop"

# Create an MQTT client instance
client = mqtt.Client(client_id=client_id)

# Set username and password for authentication
client.username_pw_set(username, password)

# Set up the connection callback functions
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT broker")
        # Subscribe to topics or perform other actions here
        client.subscribe("Chatting/")
    else:
        print(f"Connection failed with code {rc}")

def on_disconnect(client, userdata, rc):
    print("Disconnected from MQTT broker")
    
# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload.decode("utf-8")))

# Set the callback functions
client.on_connect = on_connect
client.on_disconnect = on_disconnect
client.on_message = on_message

# Connect to the MQTT broker
client.connect(shiftr_io_host, shiftr_io_port, keepalive=60)

# Start the MQTT loop (this will handle reconnecting, etc.)
client.loop_start()

# Wait for user input and publish it to the specified topic
topic = "Chatting"
while 1:
    if client.is_connected():
        message = input("")
        client.publish(topic, message)

