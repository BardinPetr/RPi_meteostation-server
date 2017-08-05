#include <SimpleTimer.h>
#include <BH1750.h>
#include "I2Cdev.h"
#include "BMP085.h"
#include <DHT11.h>
#include <Wire.h>
#include "RF.h"

SimpleTimer sendTimer;
BMP085 barometer;
DHT11 dht11(4);
BH1750 bh;

bool q = false;
int32_t lastMicros;
int getBMP_t() {
  barometer.setControl(BMP085_MODE_TEMPERATURE);
  lastMicros = micros();
  while (micros() - lastMicros < barometer.getMeasureDelayMicroseconds());
  return (int)(barometer.getTemperatureC());
}
float getBMP_p() {
  barometer.setControl(BMP085_MODE_PRESSURE_3);
  while (micros() - lastMicros < barometer.getMeasureDelayMicroseconds());
  return barometer.getPressure() * 0.00750062;
}

void readSensors()
{
  sens.lux = bh.readLightLevel();
  float nh, nt;
  dht11.read(nh, nt);
  sens.dht_hh = int(nh);
  sens.dht_tt = int(nt);
  sens.gw = analogRead(A0);
  //sens.gw = map(sens.gw, 1023, 0, 0, 100);
  sens.bmp_t = getBMP_t();
  sens.bmp_p = getBMP_p();
}

void sendUptime()
{
  digitalWrite(5, 1);
  readSensors();
  fixSensors();
  sendUptimeSerial();
  if (q) sendUptimeWIFI();
  delay(100);
  digitalWrite(5, 0);
}

void setup() {
  Serial.begin(115200);
  Wire.begin(2, 0);

  bh.begin();
  barometer.initialize();

  setupWIFI();

  sendTimer.setInterval(1500L, sendUptime);

  pinMode(5, OUTPUT);
}

void loop() {
  sendTimer.run();
  q = runWIFI(q);
}
