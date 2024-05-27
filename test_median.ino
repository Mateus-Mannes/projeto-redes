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

char nomeRede[] = "iPhone de Mateus"; // SSID da rede (usar a do celular)
char senhaRede[] = "mateusmm"; // senha da rede

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char brokerIp[] = "172.20.10.2"; // IP local público
int brokerPort = 1883; // porta do mosquitto broker
const char topic[] = "esp/devices"; // nome do tópico

int scanTime = 3; // tempo do scan
BLEScan* pBLEScan;

int N = 2; // Constante do ambiente
int baseRssi = -69; // RSSI de 1 metro

std::map<std::string, std::vector<int>> rssisPorMac; // armazena valores de RSSI separados por MAC

void setup() {
    Serial.begin(9600);
    while (!Serial) {
        ; // Espera a porta conectar
    }
    conectarRede();
    conectarBroker();
    setupScan();
}

void conectarRede() {
    Serial.print("Tentando conectar na rede SSID: ");
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    Serial.println(nomeRede);
    WiFi.begin(nomeRede, senhaRede);
    int count = 0;
    while (WiFi.status() != WL_CONNECTED) {
        // Continua até conectar
        Serial.print(".");
        delay(300);
        count++;
        if (count % 50 == 0) Serial.println();
        if (count % 200 == 0) {
            Serial.print("Dificuldade de conectar na rede, tentando novamente");
            WiFi.disconnect();
            WiFi.begin(nomeRede, senhaRede);
        }
    }
    Serial.println("Conectado na rede com sucesso!");
    Serial.println();
}

void conectarBroker() {
    Serial.print("Tentando conectar no MQTT broker: ");
    Serial.println("IP Broker: " + String(brokerIp) + " Porta: " + String(brokerPort));

    if (!mqttClient.connect(brokerIp, brokerPort)) {
        Serial.print("Erro ao conectar no broker! Código de erro = ");
        Serial.println(mqttClient.connectError());

        while (1); // Mantém o código parado se não conectar ao broker
    }
    Serial.println("Conectado ao broker com sucesso!");
    Serial.println();
    Serial.print("Endereço IP do ESP: ");
    Serial.println(WiFi.localIP());
    Serial.print("Porta local usada para MQTT: ");
    Serial.println(wifiClient.localPort());
    Serial.println();
}

void setupScan() 
{
    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
}

void loop() {
    BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
    Serial.println("Scan completado!");
    pBLEScan->clearResults(); // Limpa a memória
    mqttClient.poll(); // Mantém a conexão com o broker MQTT ativa
    delay(1000);
}

class AparelhosEscaneadosCallbacks : public BLEAdvertisedDeviceCallbacks {
    
    void onResult(BLEAdvertisedDevice aparelhoEscaneado) {
        
        if (aparelhoEscaneado.getName().empty()) return; // Scaneia apenas os devices com nome
        
        std::string macAddress = aparelhoEscaneado.getAddress().toString();
        std::string nome = aparelhoEscaneado.getName().c_str();
        int rssi = aparelhoEscaneado.getRSSI();
        rssiPorMac[macAddress].push_back(rssi);
        
        // Se houver valores de RSSI suficientes, estima distâncias
        if (rssiPorMac[macAddress].size() >= 15) {
          fload distancia = calcularDistancia(nome, macAddress);
          // Criando mensagem no formato "nome/MAC/distancia"
          std::string mensagem = nome + "/" + macAddress + "/" + String(distanciaFinal, 2)
          enviarMensagemBroker(mensagem);
          rssiPorMac[macAddress].clear(); // Limpa os valores de RSSI para o próximo scan
        }
    }

    float calcularDistancia(std::string nome, std::string macAddress) 
    {
        // Calcula média e mediana de RSSI
        float media = calculaMedia(rssiPorMac[macAddress]);
        float mediana = calculaMediana(rssiPorMac[macAddress]);

        // Converte para distâncias
        float distanciaMedia = pow(10, (baseRssi - meanRssi) / (10.0 * N));
        float distanciaMediana = pow(10, (baseRssi - medianRssi) / (10.0 * N));

        // Estimativa final de distância (média entre as duas estimativas)
        float distanciaFinal = (distanciaMedia + distanciaMediana) / 2;

        Serial.printf("Distancia calculada (%s - %s), Média: %.2f, Mediana: %.2f, Final: %.2f.\n",
            nome, macAddress, distanciaMedia, distanciaMediana, distanciaFinal);

        return distanciaFinal;
    }

    // Função auxiliar para calcular a média
    float calculaMedia(const std::vector<int>& valores) {
        int soma = 0;
        for(int i = 0; i < valores.size(); i++)
            soma += valores[i];
        return static_cast<float>(soma) / valores.size();
    }

    // Função auxiliar para calcular a mediana
    float calculaMediana(std::vector<int>& valores) {
        std::sort(valores.begin(), valores.end());
        if (valores.size() % 2 == 0) {
            return (valores[valores.size()/2 - 1] + valores[valores.size()/2]) / 2.0;
        } else {
            return valores[valores.size()/2];
        }
    }

    void enviarMensagemBroker(std::string mensagem) {
        mqttClient.beginMessage(topic);
        mqttClient.print(mensagem);
        mqttClient.endMessage();
    }

};
