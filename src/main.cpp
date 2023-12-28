#include <Arduino.h>

#include <WiFi.h>

#include "Irvine.h"
#include "Configuration.h"
#include "ESP32TimerInterrupt.h"

#define TIMER0_INTERVAL_MS        1000

ESP32Timer ITimer0(0);

Irvine irvine;

bool IRAM_ATTR TimerHandler0(void * timerNo)
{
	static bool toggle0 = false;

	//timer interrupt toggles pin PIN_D19
	digitalWrite(22, toggle0);
	toggle0 = !toggle0;

    Serial.println("ISR--------------------");
    Serial.printf("Init sm: %d\r\n", (int)comm.m_state);
    

	return true;
}

void setup()
{
  
	if (ITimer0.attachInterruptInterval(TIMER0_INTERVAL_MS * 1000, TimerHandler0))
	{
		Serial.print(F("Starting  ITimer0 OK, millis() = "));
		Serial.println(millis());
	}
	else
		Serial.println(F("Can't set ITimer0. Select another freq. or timer"));


  Serial.begin(115200);

  pinMode(21, OUTPUT); // 1wire
  pinMode(22, OUTPUT);  

  configuration.initSource();
  configuration.readConfig();
}

void loop()
{
  irvine.loop();
  configuration.cyclic();
}
