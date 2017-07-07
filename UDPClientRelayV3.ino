
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
int status = WL_IDLE_STATUS;
//char ssid[] = "linksys";
//char ssid[] = "Huybrechts";
char ssid[] = "telenet-62A08";
//char pass[]= "kweepeer";
//char pass[]= "VERDI";
char pass[]= "xSX4AfcATZ36";
int keyIndex = 0;
boolean firstRun = true; 
boolean haveRemoteIP = false;

unsigned int localPort = 2500;
unsigned int otherPort = 2600;
unsigned long lastTime;

int stateOld = LOW;
int state = LOW;
char packetBuffer[255];
char upTime[20];
char replyBuffer[] = "acknowledged";
String upTimeStr;
String message;
IPAddress serverIP;


WiFiUDP UdpReceive;
WiFiUDP UdpSend;


void setup() {
  // Open the Serial Port
Serial.begin(115200);
pinMode(0, OUTPUT);
pinMode(2, INPUT);
digitalWrite(0,HIGH);

 // Connect to the Wifi
WiFi.begin(ssid, pass);
Serial.print("Connecting");
Serial.print(ssid);
while(WiFi.status() != WL_CONNECTED){
  delay(500);
  Serial.print(".");
  }

 // Open the Udp Port

 UdpReceive.begin(localPort);
 Serial.println("");
  

}

void loop() {
  // When 1st run broadcast Hardware ID Local IP and Local Port
if (firstRun){  
  Serial.print("LocalIP: ");
  Serial.println(WiFi.localIP());
  upTimeStr = String(millis()/1000);
  message = "Report/HWid:"+String(ESP.getChipId())+"/Model:RELAY/Uptime:"+upTimeStr+"/Dop:00";
  UdpSendString(otherPort,IPAddress(192,168,255,255),message);
  Serial.println(upTimeStr);
  message = "";
  firstRun = false;
  }
/* Reconnect to Wifi network in case of Wifi loss
/if (WiFi.status() != WL_CONNECTED){
  WiFi.begin(ssid, pass);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
    }
}
*/

int noBytes = UdpReceive.parsePacket();
if (noBytes){
  UdpReceive.read(packetBuffer,noBytes);
  haveRemoteIP= true;
  Serial.println(packetBuffer);
  String str(packetBuffer);
  Serial.print("Raw receive string: "); 
  Serial.println(str);
  if (str.equals("Report")){
    Serial.println("Report Command Received");
    haveRemoteIP= true;
    serverIP = UdpReceive.remoteIP();
    upTimeStr= String(millis()/1000);
    message = "Report/HWid:" + String(ESP.getChipId())+"/Model:RELAY/Uptime:" + upTimeStr + "/Dop:02";
    Serial.println(serverIP);
    UdpSendString(otherPort,serverIP,message);
    message= "";

    }
   if(str.startsWith("Set/Dop:00/Value:")){
   if(str.equals("Set/Dop:00/Value:High")){
    digitalWrite(0,LOW); // Relay Harware relay works inverse
    Serial.println("received Set/Pin:00/State:High");
    UdpSendString(UdpReceive.remotePort(), UdpReceive.remoteIP(),"OK");
    }
   if(str.equals("Set/Dop:00/Value:Low")){
    digitalWrite(0,HIGH); // Relay Hardware relay works inverse
    Serial.println("received Set/Pin:00/State:Low");
    UdpSendString(UdpReceive.remotePort(), UdpReceive.remoteIP(),"OK");
    }
    }
    else{ // Error handling reply with string "..." if value is not High or Low
     UdpSendString(UdpReceive.remotePort(), UdpReceive.remoteIP(),"Invallid Value received for Dop"); 
      }
   if(str.equals("Get/Dop:00/Value")){
    if (digitalRead(0)==HIGH){
      UdpSendString(UdpReceive.remotePort(),UdpReceive.remoteIP(),"Low");
      }
    else if (digitalRead(0)==LOW) {
     UdpSendString(UdpReceive.remotePort(),UdpReceive.remoteIP(),"High");
    }

    }
    
   if (str.equals("OK")){
    haveRemoteIP= true;
    serverIP = UdpReceive.remoteIP();
    }
  str="";
  UdpReceive.flush();
  //clear packetBuffer
  for (int i=0; i<256; i++){
    packetBuffer[i]=0;}
    
   }

// check if we have a remote IP address
if (haveRemoteIP==false){
  if ((millis()-lastTime)>1000){
    Serial.print(".");
    firstRun= true;
    lastTime = millis();
  }
}

// Detect Event
state = digitalRead(2);
if (state != stateOld){
  
  message = "Event/HWid:"+String(ESP.getChipId())+"/Model:RELAY/Pin:02/State:";
  if (state==HIGH){
    message=message+"High";
  }
  else if (state==LOW){
    message=message+"Low";
  }
  UdpSendString(otherPort,serverIP, message);
  Serial.println("Event Detected");
  stateOld = state;
  
  message="";
  }

}
// Procedure to send UPD packet
void UdpSendString (int poort, IPAddress destinationAddress, String message){
  UdpSend.beginPacket(destinationAddress, poort);
  UdpSend.print(message);
  UdpSend.endPacket();
  UdpSend.flush();
  UdpReceive.stop();
  delay(10);
  UdpReceive.begin(localPort);
  Serial.println(message);
  }
