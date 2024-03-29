#This code connects to a private shiftr.io instance
#using python. It establishes itself as an NCAP and
#broadcasts its name on a known channel so any TIMs
#can communicate to it. Once a TIM reaches out to it
#it creates an uplink and downlink channel to that TIM.

import paho.mqtt.client as mqtt
import schedule
from dataclasses import dataclass
import tkinter as tk

# Your shiftr.io MQTT broker information
shiftr_io_host = "rusmartlabclinic.cloud.shiftr.io"
shiftr_io_port = 1883  # Default MQTT port
# Your shiftr.io credentials
username = "rusmartlabclinic"
password = "RUSmartLabClinic"
APP_ID = "1"
client_id = "APP" + APP_ID
# Create an MQTT client instance
client = mqtt.Client(client_id=client_id)
# Set username and password for authentication
client.username_pw_set(username, password)

# data class for storing channels neccessary to talk to a NCAP
@dataclass
class NCAP:
    name: str
    def uplink(self):
        return self.name + "_" + client_id
    def downlink(self):
        return client_id + "_" + self.name
    def response(self):
        return self.name + "_Server_APP_Discover_Response"
# List of all known NCAP names.
NCAP_List = []

# List of all Topics that should be subscribed to
subscriptions = [("NCAP_Server_Discover",0)]
def update_Subscriptions():
    subscriptions = [("NCAP_Server_Discover",0)]
    for NCAP in NCAP_List:
        subscriptions.append((NCAP.uplink(),0))
    client.subscribe(subscriptions)

# Set up the connection callback functions
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT broker")
        # Subscribe to topics or perform other actions here
        update_Subscriptions()
    else:
        print(f"Connection failed with code {rc}")

def on_disconnect(client, userdata, rc):
    print("Disconnected from MQTT broker")


def MessageParse(msg):
    parse = msg.split(",")
    NetSvcType = parse[0]
    NetSvcID =  parse[1]
    MsgType =  parse[2]
    MsgLength =  parse[3]
    if NetSvcType == '1': # It is a discovery service
        if NetSvcID == '4': # NCAP Discovery Request Note -> should be 4
            if MsgType == "2":
                errorCode = parse[4]
                APP_ID = parse[5]
                NCAP_ID = parse[6]
                NCAP_Name = parse[7]
                AddressType = parse[8]
                NCAP_Address = parse[9]
                return{'NetSvcType':NetSvcType, 'NetSvcID':NetSvcID, 'MsgType':MsgType, 'MsgLength':MsgLength, 'errorCode':errorCode, 'APP_ID':APP_ID, 'NCAP_ID':NCAP_ID, 'NCAP_Name':NCAP_Name, 'AddressType':AddressType, 'NCAP_Address': NCAP_Address}
        elif NetSvcID == "5":
            if MsgType == "2":
                errorCode = parse[4]
                numTims = parse[5]
                timIDs = parse[6]
                timNames = parse[7]
                return{'NetSvcType':NetSvcType, 'NetSvcID':NetSvcID, 'MsgType':MsgType, 'MsgLength':MsgLength, 'errorCode':errorCode, 'numTims':numTims, 'timIDs':timIDs, 'timNames':timNames}
        elif NetSvcID == "6":
            if MsgType == "2":
                errorCode = parse[4]
                TIM_ID = parse[5]
                NumXDCRChan = parse[6]
                XDCR_ChanIDs = parse[7]
                XDCR_ChanNames = parse[8]
                return{'NetSvcType':NetSvcType, 'NetSvcID':NetSvcID, 'MsgType':MsgType, 'MsgLength':MsgLength, 'errorCode':errorCode, 'TIM_ID': TIM_ID, 'NumChan':NumXDCRChan, 'XDCR_ChanIDs':XDCR_ChanIDs, 'XDCR_ChanNames':XDCR_ChanNames}
    elif NetSvcType == '2':
        if NetSvcID == '1':
            if MsgType == '2':
                errorCode = parse[4]
                NCAP_ID = parse[5]
                TIM_ID = parse[6]
                XDCR_ChanIDs = parse[7]
                Data = parse[8]
                Timestamp = parse[9]
                return{'NetSvcType':NetSvcType, 'NetSvcID':NetSvcID, 'MsgType':MsgType, 'MsgLength':MsgLength, 'errorCode':errorCode, 'NCAP_ID':NCAP_ID, 'TIM_ID':TIM_ID, 'XDCR_ChanIDs':XDCR_ChanIDs, 'Data':Data, 'Timestamp':Timestamp}
          

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    payload = str(msg.payload.decode("utf-8"))

    #Adds new NCAPs
    matching_NCAP = [NCAP for NCAP in NCAP_List if NCAP.name == payload]
    if(msg.topic == subscriptions[0][0] and not matching_NCAP):
        ncap = NCAP(payload)
        NCAP_List.append(ncap)
    
    #Handles messaages received from NCAPs
    for ncap in NCAP_List:
        if(msg.topic == ncap.uplink()):
            MsgDict = MessageParse(payload)
            formatted_dict = "\n".join([f"{key}: {value}\n" for key, value in MsgDict.items()])
            label3.config(text=formatted_dict)

# Set the callback functions
client.on_connect = on_connect
client.on_disconnect = on_disconnect
client.on_message = on_message

def connect():
    client.connect(shiftr_io_host, shiftr_io_port, keepalive=60)
    client.loop_start()
    label1.config(text="NCAP Status:\nConnected!")
    
def disconnect():
    client.disconnect()
    client.loop_stop()
    label1.config(text="NCAP Status:\nNot Connected!")

def downlink(event=None):
    msg = entry1.get()
    entry1.delete(0, tk.END)
    for ncap in NCAP_List:
        client.publish(ncap.downlink(), msg)

#region defineTkinter
window = tk.Tk()

window.geometry("450x500")

window.columnconfigure([0, 1, 2], minsize=150)
window.rowconfigure([0], minsize=100)
window.rowconfigure([3], minsize=40)
window.rowconfigure([1,2], minsize=10)

button1 = tk.Button(width=10, text="Connect", command=connect)
label1 = tk.Label(text="NCAP Status:\nNot Connected!")
button2 = tk.Button(width=10, text="Disconnect", command=disconnect)

label2 = tk.Label(text="Send Message to NCAP")

entry1 = tk.Entry()
entry1.bind('<Return>', downlink)

spacer1 = tk.Label()

label3 = tk.Label(text="Received\n")


button1.grid(row=0, column=0)
label1.grid(row=0, column=1)
button2.grid(row=0, column=2)
label2.grid(row=1, column=1)
entry1.grid(row=2, column=1)
spacer1.grid(row=3, column=1)
label3.grid(row=4, column=1)

#endregion

def NCAPResponse():
    if(client.is_connected):
        for ncap in NCAP_List:
            update_Subscriptions()
            client.publish(ncap.response(),client_id)
    window.after(5000, NCAPResponse)

window.after(0, NCAPResponse)
window.mainloop()

