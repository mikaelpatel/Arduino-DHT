#include "GPIO.h"
#include "DHT.h"

#if defined(ARDUINO_attiny)
#include "Software/Serial.h"
Software::Serial<BOARD::D0> Serial;
DHT11<BOARD::D1> dht;
#else
DHT11<BOARD::D7> dht;
#endif

void setup()
{
  Serial.begin(57600);
  while (!Serial);
}

void loop()
{
  float humidity, temperature;
  int res = dht.read(humidity, temperature);
  if (res > 0) {
    Serial.print(millis());
    Serial.print(F(": "));
    Serial.print(humidity);
    Serial.print(F(" RH, "));
    Serial.print(temperature);
    Serial.println(F(" C"));
  }
  else if (res < 0) {
    Serial.print(millis());
    Serial.print(F(": res = "));
    Serial.println(res);
  }
  delay(2000);
}
