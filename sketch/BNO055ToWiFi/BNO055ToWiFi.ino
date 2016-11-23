/* 
  WiFiTelnetToSerial - Example Transparent UART to Telnet Server for esp8266

  Copyright (c) 2015 Hristo Gochkov. All rights reserved.
  This file is part of the ESP8266WiFi library for Arduino environment.
 
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include <ESP8266WiFi.h>

//how many clients should be able to telnet to this ESP8266
#define MAX_SRV_CLIENTS 10
const char* ssid =            //"**********";
const char* password =        //"**********";

WiFiServer server(10023);
WiFiClient serverClients[MAX_SRV_CLIENTS];

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

/* This driver uses the Adafruit unified sensor library (Adafruit_Sensor),
   which provides a common 'type' for sensor data and some helper functions.

   To use this driver you will also need to download the Adafruit_Sensor
   library and include it in your libraries folder.

   You should also assign a unique ID to this sensor for use with
   the Adafruit Sensor API so that you can identify this particular
   sensor in any data logs, etc.  To assign a unique ID, simply
   provide an appropriate value in the constructor below (12345
   is used by default in this example).

   Connections
   ===========
   Connect SCL to analog 5
   Connect SDA to analog 4
   Connect VDD to 3.3-5V DC
   Connect GROUND to common ground

   History
   =======
   2015/MAR/03  - First release (KTOWN)
*/

/* Set the delay between fresh samples */
#define BNO055_SAMPLERATE_DELAY_MS (100)

Adafruit_BNO055 bno = Adafruit_BNO055(55);

/**************************************************************************/
/*
    Displays some basic information on this sensor from the unified
    sensor API sensor_t type (see Adafruit_Sensor for more information)
*/
/**************************************************************************/
void displaySensorDetails(void)
{
  sensor_t sensor;
  bno.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" xxx");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" xxx");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" xxx");
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}


void setup() {
  Serial1.begin(9600);
  WiFi.begin(ssid, password);
  Serial1.print("\nConnecting to "); Serial1.println(ssid);
  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED && i++ < 20) delay(500);
  if(i == 21){
    Serial1.print("Could not connect to"); Serial1.println(ssid);
    while(1) delay(500);
  }
  //start UART and the server
  //Serial.begin(9600);
  server.begin();
  server.setNoDelay(true);
  
  Serial1.print("Ready! Use 'telnet ");
  Serial1.print(WiFi.localIP());
  Serial1.println(" 10023' to connect");


    if(!bno.begin())
    {
      /* There was a problem detecting the BNO055 ... check your connections */
      Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
      //while(1);
    }
   
  delay(1000);

  /* Use external crystal for better accuracy */
  bno.setExtCrystalUse(true);
   
  /* Display some basic information on this sensor */
  displaySensorDetails();

}


void BunnyLoop(void)
{
  /* Get a new sensor event */

}

 char Buff[512];

void loop() {
  while (true) 
  {
  uint8_t i;
  //check if there are any new clients
  if (server.hasClient()){
    for(i = 0; i < MAX_SRV_CLIENTS; i++){
      //find free/disconnected spot
      if (!serverClients[i] || !serverClients[i].connected()){
        if(serverClients[i]) serverClients[i].stop();
        serverClients[i] = server.available();
        Serial1.print("New client: "); Serial1.print(i);
        continue;
      }
    }
    //no free/disconnected spot so reject
    WiFiClient serverClient = server.available();
    serverClient.stop();
  }
  //check clients for data
  for(i = 0; i < MAX_SRV_CLIENTS; i++){
    if (serverClients[i] && serverClients[i].connected()){
      while(serverClients[i].connected()) 
      {
        sensors_event_t event;
        bno.getEvent(&event);

        /* Board layout:
              +----------+
              |         *| RST   PITCH  ROLL  HEADING
          ADR |*        *| SCL
          INT |*        *| SDA     ^            /->
          PS1 |*        *| GND     |            |
          PS0 |*        *| 3VO     Y    Z-->    \-X
              |         *| VIN
              +----------+
        */
        Buff[0]=0;

        // No floats in sprintf
        //sprintf(Buff,"%f,%f,%f\n",,(float)event.orientation.y,(float)event.orientation.z);
        dtostrf((float)event.orientation.x,12,10,&Buff[strlen(Buff)]);
        strcat(Buff,",");
        dtostrf((float)event.orientation.y,12,10,&Buff[strlen(Buff)]);
        strcat(Buff,",");
        dtostrf((float)event.orientation.z,12,10,&Buff[strlen(Buff)]);
        strcat(Buff,"\n");
        
        /* The processing sketch expects data as roll, pitch, heading */
        //Serial.print(F("Orientation: "));
        //Serial.print((float)event.orientation.x);
        //Serial.print(F(" "));
        //Serial.print((float)event.orientation.y);
        //Serial.print(F(" "));
        //Serial.print((float)event.orientation.z);
        //Serial.println(F(""));
        serverClients[i].write((uint8_t *)Buff, strlen(Buff));

/*
        // Also send calibration data for each sensor.
        uint8_t sys, gyro, accel, mag = 0;
        bno.getCalibration(&sys, &gyro, &accel, &mag);
        Serial.print(F("Calibration: "));
        Serial.print(sys, DEC);
        Serial.print(F(" "));
        Serial.print(gyro, DEC);
        Serial.print(F(" "));
        Serial.print(accel, DEC);
        Serial.print(F(" "));
        Serial.println(mag, DEC);
*/
        delay(BNO055_SAMPLERATE_DELAY_MS);

      }
    }
  }
  }
}
