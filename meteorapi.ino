/*
  Web client

 This sketch connects to a website (http://www.google.com)
 using an Arduino Wiznet Ethernet shield.

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13

 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe, based on work by Adrian McEwen

 */

#include <SPI.h>
#include <Ethernet.h>
#include <OneWire.h>

OneWire ds(2);  // an pin 2
byte addr[8];

unsigned long lastConnectionTime = 0;             // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 60L * 1000L; // delay between updates, in milliseconds
// the "L" is needed to use long type numbers

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
char server[] = "sebitemperature.meteor.com";    // name address for Google (using DNS)

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 0, 177);

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // search for devices
  if ( !ds.search(addr)) {
      Serial.println("ERROR: No devices found. Exiting...");
      Serial.println("Make sure the sensor is connected (pullup necessary)");
      Serial.println("Then reset. Exiting...");
      delay(1000);
      exit(1);
  }

  Serial.print("Device with followin address found:");
  for( int i = 0; i < 8; i++) {
    Serial.print(addr[i], HEX);
    Serial.print(" ");
  }
  Serial.print("\n");
  Serial.print("\n");

  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.println("connecting...");

 
}

void loop() { 
  float temperature;
  // if there are incoming bytes available
  // from the server, read them and print them:
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }

 // if ten seconds have passed since your last connection,
  // then connect again and send data:
  if (millis() - lastConnectionTime > postingInterval) {
    temperature = getTemperature();
    httpRequest(temperature);
    
  }
}

 

float getTemperature() {
    int HighByte, LowByte, TReading, SignBit, Tc_100, Whole, Fract;
  int i;
  byte present = 0;
  byte data[12];

 ds.reset();
  ds.select(addr);
  ds.write(0x44,1);         // start Konvertierung, mit power-on am Ende

  delay(1000);     // 750ms sollten ausreichen
  // man sollte ein ds.depower() hier machen, aber ein reset tut das auch

  present = ds.reset();
  ds.select(addr);   
  ds.write(0xBE);         // Wert lesen

  for ( i = 0; i < 9; i++) {           // 9 bytes
    data[i] = ds.read();
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  //Serial.print(" CRC=");
  //Serial.print( OneWire::crc8( data, 8), HEX);
  
 LowByte = data[0];
  HighByte = data[1];
  TReading = (HighByte << 8) + LowByte;
  SignBit = TReading & 0x8000;  // test most sig bit
  if (SignBit) // negative
  {
    TReading = (TReading ^ 0xffff) + 1; // 2's comp
  }
  Tc_100 = (6 * TReading) + TReading / 4;    // mal (100 * 0.0625) oder 6.25



  Whole = Tc_100 / 100;  // Ganzzahlen und Brüche trennen
  Fract = Tc_100 % 100;

  float bla = float(Whole) + float(Fract)/100;


  if (SignBit) // negative Werte ermitteln
  {
     Serial.print("-");
     bla = bla * (-1.0);
  }
  
  Serial.print("---");
  Serial.print(bla);
  Serial.print("---");
   
  Serial.println();
  return bla;
}



// this method makes a HTTP connection to the server:
void httpRequest(float temp) {
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection:
  if (client.connect(server, 80)) {
    Serial.println("connecting...");
    // send the HTTP GET request:
    client.print("GET /api?temp=");
    client.println(temp);
    client.println("Host: sebitemperature.meteor.com");
    client.println("User-Agent: arduino-ethernet");
    client.println("Connection: close");
    client.println();

    // note the time that the connection was made:
    lastConnectionTime = millis();
  } else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
}
