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

int scanTime = 2; // tempo do scan
BLEScan* pBLEScan;

std::vector<int> rssis;
float rssiBase = 0;
std::vector<int> Ns;

void setup() {
    Serial.begin(9600);
    while (!Serial) {
        ; // Espera a porta conectar
    }
    setupScan();
}

void loop() {
    BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
    Serial.println("Scan completado!");
    pBLEScan->clearResults(); // Limpa a memória
    delay(500);
}

class AparelhosEscaneadosCallbacks : public BLEAdvertisedDeviceCallbacks {
    
    void onResult(BLEAdvertisedDevice aparelhoEscaneado) {
        
        if (aparelhoEscaneado.getName() != "mateusmannes") return; // Scaneia apenas os devices com nome

        int rssi = aparelhoEscaneado.getRSSI();
        rssis.push_back(rssi);
        Serial.printf("mateusmannes escaneado %d ", rssis.size());
        
        // calula a média do rssi de 1 metro
        if (rssis.size() == 50) {
          
          float mediaRssi = calculaMedia(rssis);
          float medianaRssi = calculaMediana(rssis);
          rssiBase = (mediaRssi + medianaRssi) / 2.0;
          Serial.printf("Média RSSI: %.2f\n", rssiBase);

          Serial.println("Afaste o esp em 3 metros. Depois, aperte qualquer tecla para continuar...");
          while (!Serial.available()) {
              delay(100); // Espera o usuário apertar uma tecla
          }
        }
        // Em seguida, calcula a média do N, para 3 metros de distancia
        else if(rssis.size() > 50 && rssis.size() <= 100) 
        {
            float N = (rssiBase - rssi) / (10 * log10(3));
            Ns.push_back(N);
        }
        else if(rssis.size() == 101) 
        {
            float mediaN = calculaMedia(Ns);
            Serial.printf("Média N: %.2f\n", mediaN);
            Ns.clear();
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

