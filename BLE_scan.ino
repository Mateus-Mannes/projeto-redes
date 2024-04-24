/*
  Baseado nos exemplos:

  ArduinoMqttClient - WiFi Simple Sender:
  https://github.com/arduino-libraries/ArduinoMqttClient/blob/master/examples/WiFiSimpleSender/WiFiSimpleSender.ino

  BLE Scan:
  https://github.com/nkolban/ESP32_BLE_Arduino/blob/master/examples/BLE_scan/BLE_scan.ino

  Wi-Fi STA Connect and Disconnect Example:
  https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/WiFiClientConnect/WiFiClientConnect.ino
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <ArduinoMqttClient.h>
#include <WiFi.h>

char ssid[] = "iPhone de Mateus";    // SSID da rede (usar a do celular)
char pass[] = "mateusmm";    // senha da rede

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char broker[] = "172.20.10.2"; // Ip local público
int        port     = 1883; // porta do mosquitto broker
const char topic[]  = "esp/devices"; // nome do tópico

int scanTime = 5; // tempo do scan
BLEScan* pBLEScan;

int N = 2; // Constante do ambiente
int baseRssi = -69; // rssi_ref 1 metro

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      
      if (!advertisedDevice.getName().empty()) // scaneia apenas os devices com nome
      { 
        int rssi = advertisedDevice.getRSSI();
        float distance = pow(10, (baseRssi - rssi) / 10.0 * N);
        Serial.printf("Name: %s, Device: %s, RSSI: %d, Distance: %.2f meters\n",
          advertisedDevice.getName().c_str(), advertisedDevice.getAddress().toString().c_str(), rssi, distance);

        Serial.print("Sending message to topic: ");
        Serial.println(topic);

        // manda a mensagem no formato -> NOME/MAC/DISTANCIA
        mqttClient.beginMessage(topic);
        String concatenatedMessage = String(advertisedDevice.getName().c_str()) + "/" +
                             advertisedDevice.getAddress().toString().c_str() + "/" +
                             String(distance);

        mqttClient.print(concatenatedMessage);
        mqttClient.endMessage();

        Serial.println();
      }

    }
};

void setup() {

  Serial.begin(9600);
  while (!Serial) {
    ; // Espera a porta conectar
  }

  // Conecta na rede
  Serial.print("Tentando conectar na rede SSID: ");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  int count = 0;
  while(WiFi.status() != WL_CONNECTED) {
    // Continua até conectar
    Serial.print(".");
    delay(300);
    count++;
    if(count % 50 == 0) Serial.println();
    if(count % 200 == 0) {
      WiFi.disconnect();
      WiFi.begin(ssid, pass);
    }
  }

  Serial.println("Conectado na rede com sucesso !");
  Serial.println();

  Serial.print("Tentando conectar no MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("Erro ao conectar no broker! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }

  Serial.println("Conectado ao broker com sucesso!");
  Serial.println();


  Serial.println("Scanning devices...");

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
}

void loop() {
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.print("Quantidade de devices: ");
  Serial.println(foundDevices.getCount());
  Serial.println("Scan completado!");
  pBLEScan->clearResults();   // limpa a memória
  
  // chama o poll para não perder a conexão com o broker
  mqttClient.poll();
  
  delay(2000);
}