// přidání potřebných knihoven
#include "DHT.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#include <ESP8266WiFi.h>

// nastavení názvu zařízení
#define DEVICE "Moje Meteostanice"

// údaje k připojení k Wi-Fi síti
const char* ssid = "HomeAP_Baklazkova"; // zde vyplňte název WiFi (SSID).
const char* password = "5555544444"; // zde vyplňte heslo k WiFi.

// údaje k připojení k MQTT brokeru
const char* mqtt_broker = "broker.emqx.io";
const int mqtt_port = 1883;
const char* mqtt_username = "emqx1";
const char* mqtt_password = "public";

// topic pro MQTT komunikaci
const char* topic = "mereni"; //zde vyplňte libovolné jméno pro 'topic'

// údaje k připojení k InfluxDB
#define INFLUXDB_URL "https://eu-central-1-1.aws.cloud2.influxdata.com"
#define INFLUXDB_TOKEN "iN9yIPWqL0YDQnlnOLI4UR8MVWxh-xvIrF7bTcOxW3Xa7RXgyaa01u0Q5Xamenft6Wlv9dFZ0AyjQTbRs3vaCg=="
#define INFLUXDB_ORG "394991d067816539"
#define INFLUXDB_BUCKET "MTP_Meteostanice"

// určení časového pásma (pro správné uložení dat do time-series databáze)
#define TZ_INFO "UTC-1"

// vytvoření instance klienta InfluxDB
InfluxDBClient clientDB(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

// inicializace objektu pro ukládání dat
Point sensor("weather");

// inicializace klienta pro Wi-Fi
WiFiClient espClient;

// inicializace klienta pro MQTT komunikaci
PubSubClient PubSubClient(espClient);

// průměrný tlak pro "Plzeň" k měření nadmořské výšky
#define PRUMER_TLAK 101100.0

// typ senzoru DHT
#define Type DHT11

// pin pro připojení senzoru DHT
int DHT11Pin = 5;

// inicializace senzorů DHT a BMP
DHT dht(DHT11Pin, Type);
Adafruit_BMP085 bmp;

// proměnné pro sledování času od posledního odeslání dat a interval odesílání dat pro MQTT
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 60000; // 1 sekunda = 1000ms

// proměnné pro ukládání hodnot: teplota, vlhkost, tlak a nadmořská výška
double t, h, p, nm = 0;

// převední 'DEVICE' do proměnné 'deviceName' pro vizualizaci názvu zařízení
const char* deviceName = DEVICE;

void setup() {
  // inicializace sériové komunikace
  Serial.begin(9600);

  // inicializace senzoru DHT
  dht.begin();

  // kontrola přítomnosti senzoru BMP180
  if (!bmp.begin()) {
    Serial.println("Nebyl nalezen senzor BMP180, zkontrolujte zapojení!");
    while (1) {}
  }

  // připojení k existující Wi-Fi síti jako klient.
  WiFi.mode(WIFI_STA);
  // spouští připojení k Wi-Fi síti pomocí zadaného jména (SSID) a hesla.
  WiFi.begin(ssid, password);

  // čekání na připojení k Wi-Fi síti
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  // informace o úspěšném připojení k Wi-Fi síti a vypsání IP adresy
  Serial.println("WiFi připojení OK!");
  Serial.println("IP adresa: ");
  Serial.println(WiFi.localIP());

  // synchronizace času s NTP (Network Time Protocol) servery
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  // nastavení serveru pro MQTT komunikaci
  PubSubClient.setServer(mqtt_broker, mqtt_port);

  // volání metody k připojení k MQTT brokeru
  reconnect();

  // kontrola připojení k InfluxDB
  if (clientDB.validateConnection()) {
    Serial.print("Připojeno k InfluxDB: ");
    Serial.println(clientDB.getServerUrl());
  } else {
    Serial.print("Připojení k InlfuxDB se nepodařilo: ");
    Serial.println(clientDB.getLastErrorMessage());
  }

  Serial.println();

  // přidává 'tagy' k měřeným datům
  sensor.addTag("device", DEVICE);
  sensor.addTag("SSID", WiFi.SSID());
}


void loop() {
  // odeslání dat pouze pokud uplynul interval 'sendInterval' od posledního odeslání
  if (millis() - lastSendTime >= sendInterval) {
    lastSendTime = millis();

    //volání meod pro měření ze senzorů
    h = dht.readHumidity();
    t = dht.readTemperature();
    p = bmp.readPressure();
    nm = bmp.readAltitude(PRUMER_TLAK);

    //převede z Pa na hPa
    p /= 100.0; 
  
    //zaokrouhlení teploty a nadmořské výšky na 2 desetinné místa
    t = round(t * 100) / 100;
    nm = round(nm * 100) / 100;


    // výpis dat do sériového monitoru
    Serial.println("");
    Serial.println("");

    Serial.print("Vlhkost: ");
    Serial.print(h);
    Serial.print(" %\t");

    Serial.print("Teplota: ");
    Serial.print(t);
    Serial.println(" °C");

    Serial.print("Tlak = ");
    Serial.print(p);
    Serial.print(" hPa\t");

    Serial.print("Nadmořská výška = ");
    Serial.print(nm);
    Serial.println(" M");

    Serial.print("Jmeno zarizeni: ");
    Serial.print(deviceName);

    Serial.println("");

    //volání funkcí pro odeslání dat do Node-RED a InfluxDB
    nodeREDFunction();
    influxDBFunction();

    Serial.println("");
    Serial.println("------------------------------------------------------------------------------");
    Serial.println("");
    Serial.println("Vyčkejte 5 minut na další měření...");
    Serial.println("");
    Serial.println("------------------------------------------------------------------------------");

    // měření se bude opakovat každých 60 sekud (1 sekunda = 1000ms)
    delay(300000);
  }
}

void nodeREDFunction() {

  // vytvoření JSON objektu
  StaticJsonDocument < 200 > jsonDocument;
  jsonDocument["teplota"] = t;
  jsonDocument["vlhkost"] = h;
  jsonDocument["tlak"] = p;
  jsonDocument["nadmorska_vyska"] = nm;
  jsonDocument["nazev_zarizeni"] = deviceName;
  jsonDocument["nazev_wifi"] = ssid;

  // převedení JSON objektu na řetězec
  char jsonBuffer[512];
  serializeJson(jsonDocument, jsonBuffer);

  // kontrola, zda je zařízení připojeno k MQTT broker
  while (!PubSubClient.connected()) {
    Serial.println("Nepodařilo se připojit k MQTT broker. Opětovné připojení...");
    reconnect();
  }

  // poslání (publish) dat do MQTT broker
  PubSubClient.publish(topic, jsonBuffer);

  // zobrazení poslané hodnoty v sériovém monitoru
  Serial.println("Data poslána do MQTT:");
  Serial.println(jsonBuffer);
  Serial.println();
}

void influxDBFunction() {
  // vymaže všechny předchozí hodnoty polí v objektu senzoru
  sensor.clearFields();

  // přidá hodnotu Received Signal Strength Indicator (RSSI) aktuální Wi-Fi sítě k datům senzoru
  sensor.addField("rssi", WiFi.RSSI());

  // přidá měřené data jako pole dat senzoru
  sensor.addField("teplota", t);
  sensor.addField("vlhkost", h);
  sensor.addField("baroTlak", p);
  sensor.addField("nadmorskaVyska", nm);
  sensor.addField("nazev_zarizeni", deviceName);

  // vypsání dat do Serial Monitoru
  Serial.print("Výpis dat do Influx: ");
  Serial.println(sensor.toLineProtocol());

  // zapíše bod senzorových dat do InfluxDB.
  if (!clientDB.writePoint(sensor)) {
    // pokud zápis do InfluxDB selže, napíše se error do Serial Monitoru
    Serial.print("InfluxDB zápis selhal: ");
    Serial.println(clientDB.getLastErrorMessage());
  }
}

// funkce pro pokus o připojení k MQTT brokeru
void reconnect() {

  // opakuje se, dokud není klient připojen
  while (!PubSubClient.connected()) {
    Serial.println("Pokus o připojení k MQTT...");

    // pokusí se připojit k MQTT brokeru
    if (PubSubClient.connect("ESP8266Client", mqtt_username, mqtt_password)) {
      // pokud se připojí, vypíše se zpráva
      Serial.println("Připojeno k MQTT broker!");
      Serial.println("");
    } else {
      // pokud selže, vypíše se chybový kód a zpráva o opětovném pokusu
      Serial.print("Selhalo, rc=");
      Serial.print(PubSubClient.state());
      Serial.println(" Opětovné připojení...");
      
      // počká 0,5 sekundy před dalším pokusem
      delay(500);
    }
  }
}


