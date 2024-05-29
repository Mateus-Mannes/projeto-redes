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
std::vector<int> Ns;

const int numeroAmostras = 50;

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
        Serial.printf("%d ", rssis.size());
        
        // calula a média do rssi de 1 metro
        if (rssis.size() == numeroAmostras) {
          
          float mediaRssi = calculaMedia(rssis);
          float medianaRssi = calculaMediana(rssis);
          rssiBase = (mediaRssi + medianaRssi) / 2.0;
          Serial.println();
          Serial.printf("RSSI base: %.2f\n", rssiBase);

          Serial.println("Afaste o esp em 3 metros. Depois, aperte qualquer tecla para continuar...");
          while (!Serial.available()) {
              delay(100); // Espera o usuário apertar uma tecla
          }
          Serial.readString();
          Serial.println("Coletando scans para N ");
        }
        // Em seguida, calcula a média do N, para 3 metros de distancia
        else if(rssis.size() > numeroAmostras && rssis.size() <= numeroAmostras * 2) 
        {
            float N = (rssiBase - rssi) / (10 * log10(3));
            if(N >= 2) Ns.push_back(N); // considera apenas valores maiores que 2
        }
        // Finaliza o cálculo do N médio
        else if(rssis.size() == (numeroAmostras * 2) + 1) 
        {
            float NBase = calculaMedia(Ns);
            Serial.println();
            Serial.printf("N base: %.2f (%d amostras maior ou iqual a 2)\n", NBase, Ns.size());
            Ns.clear();
            rssis.clear();

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

