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
#include <algorithm> // para ordenação para calcular a mediana

char ssid[] = "iPhone de Mateus"; // SSID da rede (usar a do celular)
char pass[] = "mateusmm"; // senha da rede

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char broker[] = "172.20.10.2"; // IP local público
int port = 1883; // porta do mosquitto broker
const char topic[] = "esp/devices"; // nome do tópico

int scanTime = 5; // tempo do scan
BLEScan* pBLEScan;

int N = 2; // Constante do ambiente
int baseRssi = -69; // RSSI de 1 metro

std::vector<int> rssiValues; // armazena valores de RSSI para cálculo de média e mediana

// Função auxiliar para calcular a média
float calculateMean(const std::vector<int>& rssiValues) {
    int sum = std::accumulate(rssiValues.begin(), rssiValues.end(), 0);
    return static_cast<float>(sum) / rssiValues.size();
}

// Função auxiliar para calcular a mediana
float calculateMedian(std::vector<int>& rssiValues) {
    std::sort(rssiValues.begin(), rssiValues.end());
    size: 
    size_t size = rssiValues.size();
    if (size % 2 == 0) {
        return (rssiValues[size / 2 - 1] + rssiValues[size / 2]) / 2.0;
    } else {
        return rssiValues[size / 2];
    }
}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        if (!advertisedDevice.getName().empty()) { // Scaneia apenas os devices com nome
            int rssi = advertisedDevice.getRSSI();
            rssiValues.push_back(rssi);

            // Se houver valores de RSSI suficientes, estima distâncias
            if (rssiValues.size() >= 5) { // Arbitrário para coleção de valores
                // Calcula média e mediana de RSSI
                float meanRssi = calculateMean(rssiValues);
                float medianRssi = calculateMedian(rssiValues);

                // Converte para distâncias
                float meanDistance = pow(10, (baseRssi - meanRssi) / 10.0 * N);
                float medianDistance = pow(10, (baseRssi - medianRssi) / 10.0 * N);

                // Estimativa final de distância (média entre as duas estimativas)
                float finalDistance = (meanDistance + medianDistance) / 2;

                Serial.printf("Nome: %s, RSSI: %d, Distância Média: %.2f, Distância Mediana: %.2f, Distância Final: %.2f\n",
                    advertisedDevice.getName().c_str(), rssi, meanDistance, medianDistance, finalDistance);

                // Manda mensagem MQTT com a distância estimada
                mqttClient.beginMessage(topic);
                mqttClient.print(String(advertisedDevice.getName().c_str()) + "/" +
                                 String(finalDistance, 2));
                mqttClient.endMessage();

                // Limpa os valores de RSSI para o próximo scan
                rssiValues.clear();
            }
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
    while (WiFi.status() != WL_CONNECTED) {
        // Continua até conectar
        Serial.print(".");
        delay(300);
        count++;
        if (count % 50 == 0) Serial.println();
        if (count % 200 == 0) {
            WiFi.disconnect();
            WiFi.begin(ssid, pass);
        }
    }

    Serial.println("Conectado na rede com sucesso!");
    Serial.println();

    Serial.print("Tentando conectar no MQTT broker: ");
    Serial.println(broker);

    if (!mqttClient.connect(broker, port)) {
        Serial.print("Erro ao conectar no broker! Código de erro = ");
        Serial.println(mqttClient.connectError());

        while (1); // Mantém o código parado se não conectar ao broker
    }

    Serial.println("Conectado ao broker com sucesso!");
    Serial.println();

    Serial.println("Escaneando dispositivos...");

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
    pBLEScan->clearResults(); // Limpa a memória

    // Mantém a conexão com o broker MQTT ativa
    mqttClient.poll();

    delay(2000);
}
