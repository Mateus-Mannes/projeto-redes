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

char dispositivo1[] = "Logi M550 L";
char dispositivo2[] = "Echo Buds 00TV";

char nomeRede[] = "iPhone de Mateus"; // SSID da rede (usar a do celular)
char senhaRede[] = "mateusmm"; // senha da rede

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char brokerIp[] = "172.20.10.2"; // IP local público
int brokerPort = 1883; // porta do mosquitto broker
const char topic[] = "esp/devices"; // nome do tópico

int scanTime = 1; // tempo do scan
BLEScan* pBLEScan;

float N = 4.0; // Constante do ambiente
float rssiBase = -54.0; // RSSI de 1 metro

std::map<std::string, std::vector<int>> rssisPorMac; // armazena valores de RSSI separados por MAC

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        ; // Espera a porta conectar
    }
    Serial.println();
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

void loop() {
    BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
    Serial.println("Scan completado!");
    pBLEScan->clearResults(); // Limpa a memória
    mqttClient.poll(); // Mantém a conexão com o broker MQTT ativa
}

class AparelhosEscaneadosCallbacks : public BLEAdvertisedDeviceCallbacks {
    
    void onResult(BLEAdvertisedDevice aparelhoEscaneado) {
        
        // Ignora outros dispositivos 
        if (aparelhoEscaneado.getName() != dispositivo1 &&
            aparelhoEscaneado.getName() != dispositivo2) return; 

        std::string macAddress = aparelhoEscaneado.getAddress().toString().c_str();
        std::string nome = aparelhoEscaneado.getName().c_str();
        int rssi = aparelhoEscaneado.getRSSI();
        rssisPorMac[macAddress].push_back(rssi);
        
        // Se houver valores de RSSI suficientes, estima distâncias
        if (rssisPorMac[macAddress].size() >= 7) {
          float distancia = calcularDistancia(nome, macAddress);
          // Criando mensagem no formato "nome/MAC/distancia"
          std::string mensagem = "Nome: " + nome + "\n" +
              "MAC: " + macAddress + "\n" + 
              "Distância: " + String(distancia, 2).c_str() + "m";
          enviarMensagemBroker(mensagem);
          rssisPorMac[macAddress].clear(); // Limpa os valores de RSSI para o próximo scan
        }
        else 
        {
          Serial.printf("(%s - %s) ", nome.c_str(), macAddress.c_str());
        }
    }

    float calcularDistancia(std::string nome, std::string macAddress) 
    {
        // Calcula média e mediana de RSSI
        float media = calculaMedia(rssisPorMac[macAddress]);
        float mediana = calculaMediana(rssisPorMac[macAddress]);

        // Converte para distâncias
        float distanciaMedia = pow(10, (rssiBase - media) / (10.0 * N));
        float distanciaMediana = pow(10, (rssiBase - mediana) / (10.0 * N));

        // Estimativa final de distância (média entre as duas estimativas)
        float distanciaFinal = distanciaMediana;//(distanciaMedia + distanciaMediana) / 2;

        Serial.println();
        Serial.printf("Distancia calculada (%s - %s), Média: %.2f, Mediana: %.2f, Final: %.2f.\n",
            nome.c_str(), macAddress.c_str(), distanciaMedia, distanciaMediana, distanciaFinal);

        return distanciaFinal;
    }

    // Função auxiliar para calcular a média
    float calculaMedia(std::vector<int>& valores) {
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
        mqttClient.print(mensagem.c_str());
        mqttClient.endMessage();
    }

};

void setupScan() 
{
    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new AparelhosEscaneadosCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
}
