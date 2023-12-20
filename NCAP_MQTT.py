#This code connects to a private shiftr.io instance
#using python. It establishes itself as an NCAP and
#broadcasts its name on a known channel so any TIMs
#can communicate to it. Once a TIM or APP reaches out to it
#it creates an uplink and downlink channel to that device.

import paho.mqtt.client as mqtt
import schedule
import _thread
from threading import Event
from dataclasses import dataclass
from random import randint, randrange
import socket

hostname = socket.gethostname()
Address= socket.gethostbyname(hostname)
AddressType = "1"

# Your shiftr.io MQTT broker information
shiftr_io_host = "rusmartlabclinic.cloud.shiftr.io"
shiftr_io_port = 1883  # Default MQTT port
# Your shiftr.io credentials
username = "rusmartlabclinic"
password = "RUSmartLabClinic"
#NCAP_ID = str(randint(1, 9999))
NCAP_ID = "1"
client_id = "NCAP"+NCAP_ID
# Create an MQTT client instance
client = mqtt.Client(client_id=client_id)
# Set username and password for authentication
client.username_pw_set(username, password)

# data class for storing channels neccessary to talk to a TIM
@dataclass
class TIM:
    name: str
    event: Event = Event()
    def uplink(self):
        return self.name + "_" + client_id
    def downlink(self):
        return client_id + "_" + self.name
# List of all known TIM names.
TIM_List = []

# data class for storing channels neccessary to talk to an APP
@dataclass
class APP:
    name: str
    def uplink(self):
        return client_id + "_" + self.name
    def downlink(self):
        return self.name + "_" + client_id
# List of all known APP names.
APP_List = []

# List of all Topics that should be subscribed to
subscriptions = [(client_id+"_Server_TIM_Discover_Response",0),(client_id+"_Server_APP_Discover_Response",0)]

def update_Subscriptions():
    subscriptions = [(client_id+"_Server_TIM_Discover_Response",0),(client_id+"_Server_APP_Discover_Response",0)]
    for TIM in TIM_List:
        subscriptions.append((TIM.uplink(),0))
    for APP in APP_List:
        subscriptions.append((APP.downlink(),0))
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

# Parsing Engine
def MessageParse(msg):
    print(msg)
    parse = msg.split(",")
    print(parse)
    NetSvcType = parse[0]
    NetSvcID =  parse[1]
    MsgType =  parse[2]
    MsgLength =  parse[3]
    if NetSvcType == '1': # It is a discovery service
        if NetSvcID == '4': # NCAP Discovery Request Note -> should be 4
            APP_ID = parse[4]
            Timeout = parse[5]
            return{'NetSvcType':NetSvcType, 'NetSvcID':NetSvcID, 'MsgType':MsgType, 'MsgLength':MsgLength, 'APP_ID':APP_ID, 'Timeout':Timeout}
        elif NetSvcID == "5":
            NCAP_ID = parse[4]
            Timeout = parse[5]
            return{'NetSvcType':NetSvcType, 'NetSvcID':NetSvcID, 'MsgType':MsgType, 'MsgLength':MsgLength, 'NCAP_ID':NCAP_ID, 'Timeout':Timeout}
        elif NetSvcID == "6":
            if MsgType == "1":
                NCAP_ID = parse[4]
                TIM_ID = parse[5]
                Timeout = parse[6]
                return{'NetSvcType':NetSvcType, 'NetSvcID':NetSvcID, 'MsgType':MsgType, 'MsgLength':MsgLength, 'NCAP_ID':NCAP_ID,  'TIM_ID':TIM_ID, 'Timeout':Timeout}
            elif MsgType == "2":
                errorCode = parse[4]
                TIM_ID = parse[5]
                NumXDCRChan = parse[6]
                XDCR_ChanIDs = parse[7]
                XDCR_ChanNames = parse[8]
                return{'NetSvcType':NetSvcType, 'NetSvcID':NetSvcID, 'MsgType':MsgType, 'MsgLength':MsgLength, 'errorCode':errorCode, 'TIM_ID': TIM_ID, 'NumXDCRChan':NumXDCRChan, 'XDCR_ChanIDs':XDCR_ChanIDs, 'XDCR_ChanNames':XDCR_ChanNames}
    elif NetSvcType == '2':
        if NetSvcID == '1':
            if MsgType == '1':
                NCAP_ID = parse[4]
                TIM_ID = parse[5]
                XDCR_ChanIDs = parse[6]
                Mode = parse[7]
                Timeout = parse[8]
                return{'NetSvcType':NetSvcType, 'NetSvcID':NetSvcID, 'MsgType':MsgType, 'MsgLength':MsgLength, 'NCAP_ID':NCAP_ID, 'TIM_ID':TIM_ID, 'XDCR_ChanIDs':XDCR_ChanIDs, 'Timeout':Timeout, 'Mode':Mode}
            elif MsgType == '2':
                errorCode = parse[4]
                NCAP_ID = parse[5]
                TIM_ID = parse[6]
                XDCR_ChanIDs = parse[7]
                Data = parse[8]
                Timestamp = parse[9]
                return{'NetSvcType':NetSvcType, 'NetSvcID':NetSvcID, 'MsgType':MsgType, 'MsgLength':MsgLength, 'errorCode':errorCode, 'NCAP_ID':NCAP_ID, 'TIM_ID':TIM_ID, 'XDCR_ChanIDs':XDCR_ChanIDs, 'Data':Data, 'Timestamp':Timestamp}
            
response = None
# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    payload = str(msg.payload.decode("utf-8"))
    topic = msg.topic

    #region New Channels
    #used to establish new channels on the mqtt broker for full duplex communication
    matching_TIM = [TIM for TIM in TIM_List if TIM.name == payload]
    if(topic == subscriptions[0][0] and not matching_TIM):
        tim = TIM(payload)
        TIM_List.append(tim)
    matching_APP = [APP for APP in APP_List if APP.name == payload]
    if(topic == subscriptions[1][0] and not matching_APP):
        App = APP(payload)
        APP_List.append(App)
    #endregion

    #region ResponseTopic
    #Sets the repsonse topic to whatever device sent the msg
    ResponseTopic = None
    for tim in TIM_List:
        if(topic == tim.uplink()):
            ResponseTopic = tim.downlink()
    for app in APP_List:
        if(topic == app.downlink()):
            ResponseTopic = app.uplink()
    #endregion

    if(topic != subscriptions[0][0] and topic != subscriptions[1][0]):
        try:
            MsgDict = MessageParse(payload)
        except:
            print("Message not recognized")
        else:
            global response
            print(MsgDict)
            if MsgDict["NetSvcType"] == '1':
                if MsgDict["NetSvcID"] == '4':
                    _thread.start_new_thread(Thread142, (tuple(MsgDict.items()), ResponseTopic))
                elif MsgDict["NetSvcID"] == '5':
                    _thread.start_new_thread(Thread152, (tuple(MsgDict.items()), ResponseTopic))
                elif MsgDict["NetSvcID"] == '6':
                    if MsgDict["MsgType"] == '1':
                        _thread.start_new_thread(Thread162, (tuple(MsgDict.items()), payload, ResponseTopic, TIM_List[int(MsgDict["TIM_ID"])].event))
                    elif MsgDict["MsgType"] == '2':
                        response = payload
                        try:
                            TIM_List[int(MsgDict["TIM_ID"])].event.set()
                        except:
                            print("No TIM found with that ID")
            elif MsgDict["NetSvcType"] == '2':
                if MsgDict["NetSvcID"] == '1':
                    if MsgDict["MsgType"] == '1':
                        _thread.start_new_thread(Thread212, (tuple(MsgDict.items()), payload, ResponseTopic, TIM_List[int(MsgDict["TIM_ID"])].event))
                    elif MsgDict["MsgType"] == '2':
                        response = payload
                        try:
                            TIM_List[int(MsgDict["TIM_ID"])].event.set()
                        except:
                            print("No TIM found with that ID")

# Use this comment structure for service threads
'''
X.X.X Service_Name (XX XX) - X

 - Description

Network Service type (NetSvcType) = _ ()
Network Service ID   (NetSvcId)   = _ ()
Message Type         (MsgType)    = _ ()
'''

# Service threads

'''
10.1.4 NCAP discovery (01 04) - Reply

 - The APP uses this service to discover active NCAPs

Network Service type (NetSvcType) = 1 (Discovery service)
Network Service ID   (NetSvcId)   = 4 (NCAPDiscovery)
Message Type         (MsgType)    = 2 (Reply)

Notes
- should be called Thread142
'''
def Thread142(MSG_Tuple, topic):
    # NCAP Server Discover Response
    #print(MSG_Tuple)
    #print(SenderInfo)
    MSG = dict(MSG_Tuple)
    #print(MSG)
    response = '1,4,2,25,0,' + MSG["APP_ID"] + ',' + NCAP_ID + ',' + client_id + ',' + AddressType + ',' + Address
    client.publish(topic, response)

'''
10.1.5 NCAP TIM discovery (01 05) - Reply

 - The APP uses this service to discover TIMs connected to active NCAPs

Network Service type (NetSvcType) = 1 (Discovery service)
Network Service ID   (NetSvcId)   = 5 (NCAPTIMDiscovery)
Message Type         (MsgType)    = 2 (Reply)
'''
def Thread152(MSG_Tuple, topic):
    MSG = dict(MSG_Tuple)
    TIM_NAMES = [TIM.name for TIM in TIM_List]
    TIM_IDS = [str(index) for index, tim_name in enumerate(TIM_NAMES)]
    response = '1,5,2,39,0,' + str(len(TIM_List)) + ',' + ';'.join(TIM_IDS) + ',' + ';'.join(TIM_NAMES)
    client.publish(topic, response)

'''
10.1.6 NCAP TIM transducer discovery (01 06) - Reply

 - The APP uses this service to discover TransducerChannels on TIMS connected to active NCAPS

Network Service type (NetSvcType) = 1 (Discovery service)
Network Service ID   (NetSvcId)   = 6 (NCAPTIMTransducerDiscovery)
Message Type         (MsgType)    = 2 (Reply)
'''
def Thread162(MSG_Tuple, payload, topic, event):
    MSG = dict(MSG_Tuple)
    #response = '1,6,2,55,' + '0,' + NCAP_ID + ',' + MSG["TIM_ID"] + ',' + NumChan + ',' + XDCR_ChanIDs_Array + ',' + XDCR_ChanNameArray
    client.publish(TIM_List[int(MSG["TIM_ID"])].downlink(), payload)
    event.wait()
    client.publish(topic, response)
    event.clear()


# '''
# 10.2.1 Synchronous read transducer sample data from a channel of a TIM (02 01) - Reply

#  - This service is used to read transducer sample data from a TIM Channel

# Network Service type (NetSvcType) = 2 (Transducer access service)
# Network Service ID   (NetSvcId)   = 1 (SyncReadTransducerSampleDataFromAChannelOfATIM)
# Message Type         (MsgType)    = 2 (Reply)
# '''
def Thread212(MSG_Tuple, payload, topic, event):
    MSG = dict(MSG_Tuple)
    client.publish(TIM_List[int(MSG["TIM_ID"])].downlink(), payload)
    event.wait()
    client.publish(topic, response)
    event.clear()

# '''
# 10.2.7 Synchronous write transducer sample data to a channel of a TIM service (02 07) - Reply

#  - This service is used to write a single transducer setting to a specific channel of a 
#    specific TIM of a specific active NCAP

# Network Service type (NetSvcType) = 2 (Transducer access service)
# Network Service ID   (NetSvcId)   = 7 (SyncWriteTransducerSampleDataFromAChannelOfATIM)
# Message Type         (MsgType)    = 2 (Reply)
# '''
# def Thread272(MSG_Tuple, SenderInfo):
#     MSG = dict(MSG_Tuple)
#     if MSG["TIM_ID"] == '1':
#         if MSG["XDCR_ChanIDs"] == '2':
#             print(str(MSG["WriteActuatorData"]))
#             display.show(MSG["WriteActuatorData"])
#     response = '2,7,2,19,0,' + NCAP_ID + ',' + TIM_ID + ',' + MSG["XDCR_ChanIDs"]
#     #publish.single(ResponseTopic, response, hostname=mqttBroker)
#     LocalResponseTopic = "RUSMARTLAB/"+MSG["APP_ID"]
#     print("Local Reponse Topic: " + LocalResponseTopic + "\n")
#     publish.single(LocalResponseTopic, response, hostname=mqttBroker)

# '''
# 10.4.1 Subscribe new TIM from NCAP (04 01) - Reply

#  - When a new TIM is plugged in, the TIM sends a TIM notification to the NCAP, and the NCAP
#    should report this new TIM to the APP. The APP should subscribe a new TIM plug-in service

# Network Service type (NetSvcType) = 4 (Event notification service)
# Network Service ID   (NetSvcId)   = 1 (SubscribeNewTIMFromNCAP)
# Message Type         (MsgType)    = 2 (Reply)
# '''
# def Thread412(MSG_Tuple, SenderInfo):
#     MSG = dict(MSG_Tuple)
#     if MSG["APP_ID"] == "1":
#         global AlertEnable
#         AlertEnable = True
#     response = '4,1,2,29,0,' + MSG["APP_ID"] + ',' + "11" + ',' + NCAP_ID + ',' + TIM_ID + ',' + '1,' + MSG["XDCR_ChanIDs"]
#     #publish.single(ResponseTopic, response, hostname=mqttBroker)
#     LocalResponseTopic = "RUSMARTLAB/"+MSG["APP_ID"]
#     print("Local Reponse Topic: " + LocalResponseTopic + "\n")
#     publish.single(LocalResponseTopic, response, hostname=mqttBroker)

# '''
# X.X.X Service_Name (XX XX) - X

#  - Description

# Network Service type (NetSvcType) = _ ()
# Network Service ID   (NetSvcId)   = _ ()
# Message Type         (MsgType)    = _ ()

# Notes
# - What is this?
# '''
# def Thread422(MSG_Tuple, SenderInfo):
#     MSG = dict(MSG_Tuple)
#     if MSG["APP_ID"] == "1":
#         global AlertEnable
#         AlertEnable = False
#     response = '4,2,2,29,0,' + MSG["APP_ID"] + ',' + "11" + ',' + NCAP_ID + ',' + TIM_ID + ',' + '1,' + MSG["XDCR_ChanIDs"]
#     #publish.single(ResponseTopic, response, hostname=mqttBroker)
#     LocalResponseTopic = "RUSMARTLAB/"+MSG["APP_ID"]
#     print("Local Reponse Topic: " + LocalResponseTopic + "\n")
#     publish.single(LocalResponseTopic, response, hostname=mqttBroker)


# Set the callback functions
client.on_connect = on_connect
client.on_disconnect = on_disconnect
client.on_message = on_message

# Connect to the MQTT broker
client.connect(shiftr_io_host, shiftr_io_port, keepalive=60)
# Start the MQTT loop (this will handle reconnecting, etc.)
client.loop_start()

def joinMessage():
    if(client.is_connected):
        update_Subscriptions()
        client.publish(("NCAP_Server_Discover"), client_id)

schedule.every(5).seconds.do(joinMessage)


while True:
    schedule.run_pending()
    
    
