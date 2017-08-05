#define __min(a,b) ((a)<(b)?(a):(b))
#define __max(a,b) ((a)>(b)?(a):(b))

#define UCODE 3526

#include <ESP8266WiFi.h>

#define MAX_SRV_CLIENTS 100
const char* ssid = "THE NULL";
const char* password = "1!QwertY!1";

WiFiServer server(23);
WiFiClient serverClients[MAX_SRV_CLIENTS];

String s;

struct Sensors
{
  int lux;
  int dht_tt;
  int dht_hh;
  int bmp_t;
  int bmp_p;
  int gw;
  int dp;
  int t;
} sens;

double dewPoint(double celsius, double humidity)
{
  double a = 17.271;
  double b = 237.7;
  double temp = (a * celsius) / (b + celsius) + log(humidity * 0.01);
  double Td = (b * temp) / (a - temp);
  return Td;
}

double dewPoint2(double celsius, double humidity)
{
  double RATIO = 373.15 / (273.15 + celsius);  // RATIO was originally named A0, possibly confusing in Arduino context
  double SUM = -7.90298 * (RATIO - 1);
  SUM += 5.02808 * log10(RATIO);
  SUM += -1.3816e-7 * (pow(10, (11.344 * (1 - 1 / RATIO ))) - 1) ;
  SUM += 8.1328e-3 * (pow(10, (-3.49149 * (RATIO - 1))) - 1) ;
  SUM += log10(1013.246);
  double VP = pow(10, SUM - 3) * humidity;
  double T = log(VP / 0.61078); // temp var
  return (241.88 * T) / (17.558 - T);
}

void fixSensors() {
  sens.lux = _max(0, _min(99999, sens.lux));
  sens.dht_tt = _max(0, _min(90, sens.dht_tt));
  sens.bmp_t = _max(0, _min(90, sens.bmp_t));
  sens.bmp_p = _max(0, _min(9999, sens.bmp_p));

  sens.t = sens.dht_tt + sens.bmp_t;
  sens.t = sens.t / 2;

  sens.dp = (int)dewPoint(sens.t, sens.dht_tt);
}

void sendUptimeSerial()
{
  s = "@";
  s += (sens.gw);
  s += (";");
  s += (sens.t);
  s += (";");
  s += (sens.dht_hh);
  s += (";");
  s += (sens.lux);
  s += (";");
  s += (sens.bmp_p);
  s += (";");
  s += (sens.dp);
  s += ("\n");
  Serial.print(s);
}

void sendUptimeWIFI() {
  for (int i = 0; i < MAX_SRV_CLIENTS; i++) {
    if (serverClients[i] && serverClients[i].connected()) {
      size_t len = s.length();
      uint8_t sbuf[len];
      s.getBytes(sbuf, len);
      sbuf[len - 1] = '#';
      serverClients[i].write(sbuf, len);
      delay(1);
    }
  }
}

bool setupWIFI() {
  WiFi.begin(ssid, password);
  Serial.print("\nConnecting to "); Serial.println(ssid);
  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED && i++ < 20) delay(500);
  if (i == 21) {
    Serial.print("Could not connect to"); Serial.println(ssid);
    while (1) delay(500);
  }
  server.begin();
  server.setNoDelay(true);

  Serial.print("Ready! IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("###########################################################\n");
}

bool runWIFI(bool q) {
  uint8_t i;
  uint8_t cc = 0;
  for (i = 0; i < MAX_SRV_CLIENTS; i++) {
    if (!serverClients[i] || !serverClients[i].connected()) {
      if (serverClients[i]) serverClients[i].stop();
      if (server.hasClient()) {
        serverClients[i] = server.available();
        Serial.print("New client: "); Serial.print(i);
      }
      continue;
    }
    else {
      cc ++;
    }
  }

  if (server.hasClient()) {
    WiFiClient serverClient = server.available();
    serverClient.stop();
  }

  for (i = 0; i < MAX_SRV_CLIENTS; i++) {
    if (serverClients[i] && serverClients[i].connected()) {
      if (serverClients[i].available()) {
        Serial.print("NEW REQUEST: ");
        String req = "";
        while (serverClients[i].available())
          req += String((char)serverClients[i].read());
        Serial.println(req);
        if (req.indexOf("get") != -1) {
          sendUptimeWIFI();
        }
        else if (req.indexOf("ping") != -1) {
          serverClients[i].write("pong", 4);
        }
        else if (req.indexOf("all") != -1) {
          q = true;
        }
        else if (req.indexOf("c") != -1) {
          String ab = String(cc);
          size_t len = ab.length();
          uint8_t sbuf[len];
          ab.getBytes(sbuf, len);
          serverClients[i].write(sbuf, len);
        }
      }
    }
  }
  return q;
}

/*
  void sendData(unsigned long cId, unsigned long pId, unsigned long data) {
  //     Protocol:
  //      cmdID - 1b
  //      pkgID - 2b
  //      data  - 4b

  unsigned long a = 0;
  a  = (cId + 1) * 1000000;
  a += pId * 10000;
  a += data;

  rpi.send(a, 24);
  Serial.println(a);
  delay(300);
  }

  int sendInit() {
  int rnd = random(1, 99);
  sendData(0, rnd, UCODE);
  delay(300);

  return rnd;
  }

  void sendUptimeRF() {
  int rnd = sendInit();
  sendData(1, rnd, sens.gw);
  sendData(2, rnd, sens.t);
  sendData(3, rnd, sens.dht_hh);
  sendData(4, rnd, sens.lux);
  sendData(5, rnd, sens.bmp_p);
  }

  void setupRF(int pin = 10) {
  rpi.enableTransmit(pin);
  randomSeed(analogRead(0));
  }

  void runRF() {
  }
*/
