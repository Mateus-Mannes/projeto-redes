/*
  Baseado nos exemplos:

  BLE Scan:
  https://github.com/nkolban/ESP32_BLE_Arduino/blob/master/examples/BLE_scan/BLE_scan.ino

*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <algorithm> 

int scanTime = 1; // tempo do scan
BLEScan* pBLEScan;

std::vector<int> rssis;
float rssiBase = 0;

float desvioN1 = 0;
float desvioN2 = 0;
float desvioN3 = 0;
float desvioN4 = 0;
float desvioN5 = 0;

const int numeroAmostras = 50;

// Nomes dos dispositivos
// Echo Buds 00TV
// Logi M550 L
// Wave Keys 670

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        ; // Espera a porta conectar
    }
    setupScan();
    Serial.println();
    Serial.println("Coletando scans para RSSI");
}

void loop() {
    BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
    pBLEScan->clearResults(); // Limpa a memória
}

class AparelhosEscaneadosCallbacks : public BLEAdvertisedDeviceCallbacks {
    
    void onResult(BLEAdvertisedDevice aparelhoEscaneado) {
        
        if (aparelhoEscaneado.getName() != "Echo Buds 00TV") return; // Scaneia apenas os devices com nome específico

        int rssi = aparelhoEscaneado.getRSSI();
        rssis.push_back(rssi);
        Serial.printf("%d ", rssi);
        
        // calula a média do rssi de 1 metro
        if (rssis.size() == numeroAmostras) {
          
          float mediaRssi = calculaMedia(rssis);
          float medianaRssi = calculaMediana(rssis);
          rssiBase = (mediaRssi + medianaRssi) / 2.0;
          Serial.println();
          Serial.printf("RSSI base: %.2f\n", rssiBase);

          Serial.println("Afaste o esp em 5 metros. Depois, aperte qualquer tecla para continuar...");
          while (!Serial.available()) {
              delay(100); // Espera o usuário apertar uma tecla
          }
          Serial.readString();
          Serial.println("Coletando scans para N ");
        }
        // Em seguida, calcula os desvios de distância para as 5 opções de N
        else if(rssis.size() > numeroAmostras && rssis.size() <= numeroAmostras * 2) 
        {
            desvioN1 += abs(pow(10, (rssiBase - rssi) / (10.0 * 2.0)) - 5.0);
            desvioN2 += abs(pow(10, (rssiBase - rssi) / (10.0 * 2.5)) - 5.0);
            desvioN3 += abs(pow(10, (rssiBase - rssi) / (10.0 * 3.0)) - 5.0);
            desvioN4 += abs(pow(10, (rssiBase - rssi) / (10.0 * 3.5)) - 5.0);
            desvioN5 += abs(pow(10, (rssiBase - rssi) / (10.0 * 4.0)) - 5.0);
        }
        // Finaliza mostrando os desvios
        else if(rssis.size() == (numeroAmostras * 2) + 1) 
        {
            Serial.println();
            Serial.printf("Soma dos desvios para N1 (2.0): %.2f \n", desvioN1);
            Serial.printf("Soma dos desvios para N2 (2.5): %.2f \n", desvioN2);
            Serial.printf("Soma dos desvios para N3 (3.0): %.2f \n", desvioN3);
            Serial.printf("Soma dos desvios para N4 (3.5): %.2f \n", desvioN4);
            Serial.printf("Soma dos desvios para N5 (4.0): %.2f \n", desvioN5);
            
            // limpa variáveis
            rssis.clear();
            desvioN1 = 0;
            desvioN2 = 0;
            desvioN3 = 0;
            desvioN4 = 0;
            desvioN5 = 0;
            

            Serial.println("Aperte qualquer tecla para rodar novamente...");
            while (!Serial.available()) {
                delay(100);
            }
            Serial.readString();
        }
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

