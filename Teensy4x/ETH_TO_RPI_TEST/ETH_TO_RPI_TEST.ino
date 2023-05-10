/*
 Using Tindie development board for Teensy4.1
 created 18 Dec 2009 by David A. Mellis
 modified 9 Apr 2012 by Tom Igoe, based on work by Adrian McEwen
 modified 5 Apr 2023 by James Knea, for use with my H.A.L.O. project
 */

#include <NativeEthernet.h>

#define SERIAL_BAUD                   115200
#define PORT                          8888
#define BUFF_SIZE                     24
#define DELAY_MS                      1

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  // This ccontroller address is made up, but newer Ethernet shields have a MAC address printed on a sticker on the shield
IPAddress server(192,168,5,1); // IP address of the RPi WiFi AP
EthernetClient client;  // Initialize the Ethernet client library

uint8_t buffer[BUFF_SIZE] = {0};
int bytes_written = 0;  // variables for writing data
uint8_t count = 0;      // variables for writing data

void setup() {
  Serial.begin(115200);
  delay(2000);
  initETH();
  delay(2000);
}


void loop() {
  for (int i = 0; i < BUFF_SIZE; i++) {
    buffer[i] = i + count;
  }
  count++;
  if (count == 200) 
  {
    count = 0;
  }  
  bytes_written = client.write(buffer, BUFF_SIZE);  // write data to the server
  if (bytes_written != BUFF_SIZE) 
  {
    Serial.println("did not send all bytes");
  }
  delay(DELAY_MS);
}



void initETH()
{
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
  }
  else {
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }

  delay(1000);
  Serial.print("connecting to ");
  Serial.print(server);
  Serial.println("...");

  if (client.connect(server, PORT)) {
    Serial.print("connected to ");
    Serial.println(client.remoteIP());
  } 
  else 
  {
    Serial.println("connection failed");
  }
}