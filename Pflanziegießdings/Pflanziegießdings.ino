﻿/*
 Name:		Pflanziegießdings.ino
 Created:	15.05.2018 18:26:40
 Author:	manni
*/

//Blynk Auth Token
//char auth[] = "YourAuthToken";

// Your WiFi credentials.
// Set password to "" for open networks.
//char ssid[] = "YourNetworkName";
//char pass[] = "YourPassword";

//Meien WiFi Zugangsdaten, Datei ist in .gitignore 
#include "Credentials.h"

#define BLYNK_PRINT Serial


#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "libraries/QuickStats/QuickStats.h"

//Sensorkalibrierung
const uint8_t HumidityAlert = 0; //Wert für Blynk-Pushnotification (todo) 
const uint32_t HumidityMax = 100; //Sensorwert in Wasser
const uint32_t HumidityMin = 1023; //Sensorwert in Luft
const uint32_t humidityReadInverval = 36e+8; //Sensorwerterfassung alle 5min, Wert in µs
const uint8_t numSensorReads = 20;

BlynkTimer timer;
QuickStats stats;



void setup()
{
	// Debug console
	Serial.begin(9600);
	Blynk.begin(auth, ssid, pass);
	pinMode(D1, OUTPUT);
	//Wenn SleepMode deaktiviert dann per Timerinterval
	//Sensorwert lesen (RST und D0 müssen gebrückt sein für Wakeup) 
	timer.setInterval(humidityReadInverval/1000, run);
	run();
}

void loop()
{
	Blynk.run();
	timer.run();
}

void run() {
	readHumiditySensor(D1, A0, V1); //Sensorwert Lesen nach wakeup
	//readHumiditySensor(D2, A0, V1);
	sleep();
}

void readHumiditySensor(uint8_t EnablePin, uint8_t SensorPin, uint8_t VirtualPin) {	
	digitalWrite(EnablePin, HIGH);	delay(10); //Sensor aktivieren
	float SensorReadings[numSensorReads];
	for (int i = 0; i < numSensorReads; i++) {
		SensorReadings[i] = (float) analogRead(SensorPin); 
		delay(10);
	}

	float medianRawVal = stats.median(SensorReadings, numSensorReads);

	uint8_t val = map(medianRawVal, HumidityMin, HumidityMax, 0UL, 100UL); //Sensorwert lesen und in Wert 0% - 100% umwandeln
	BLYNK_LOG2("Raw sensor data: ", val);
	BLYNK_LOG2("Pushing data: ", val); //Wert an Blynk-Server pushen
	Blynk.virtualWrite(VirtualPin, val);
	digitalWrite(EnablePin, LOW);	//Sensor wieder deaktivieren
}

void sleep() {
	BLYNK_LOG("Gute Nacht");
	ESP.deepSleep(humidityReadInverval); //Mikrocontroller für festgelegte Zeit in Tiefschlaf versetzen
}