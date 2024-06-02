#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <algorithm>
#include <vector>

char nomeDispositivo[] = "Logi M550 L";
// Nomes dos dispositivos
// Echo Buds 00TV
// Logi M550 L
// Wave Keys 670

int scanTime = 1; // tempo do scan
BLEScan* pBLEScan;

std::vector<int> rssis;

const int numeroAmostras = 35;
const int maxDistancia = 6;
const int minRssiBase = 30;
const int maxRssiBase = 85;
const int numRssiBases = maxRssiBase - minRssiBase + 1;
const int numN = 5;
const float Ns[numN] = {2.0, 2.5, 3.0, 3.5, 4.0};

float desvios[numRssiBases][numN] = {0};

float distanciaAtual = 1.0;

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        ; // Espera a porta conectar
    }
    setupScan();
    Serial.println();
    esperarPosicionamento();
    Serial.println("Coletando scans para RSSI");
}

void loop() {
    BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
    pBLEScan->clearResults(); // Limpa a memória
}

void esperarPosicionamento() {
    Serial.println("Afaste o esp. Depois, aperte qualquer tecla para continuar...");
    while (!Serial.available()) {
        delay(100); // Espera o usuário apertar uma tecla
    }
    Serial.readString();
    Serial.println("Coletando scans para a próxima distância ");
}

class AparelhosEscaneadosCallbacks : public BLEAdvertisedDeviceCallbacks {
    
    void onResult(BLEAdvertisedDevice aparelhoEscaneado) {
        
        if (aparelhoEscaneado.getName() != nomeDispositivo) return; // Scaneia apenas os devices com nome específico

        int rssi = aparelhoEscaneado.getRSSI();
        rssis.push_back(rssi);
        Serial.printf("%d ", rssi);
        
        // Quando tiver coletado o número de amostras necessário
        if (rssis.size() == numeroAmostras) {
            // Calcular todos os desvios e somar no vetor
            for (int baseIndex = 0; baseIndex < numRssiBases; ++baseIndex) {
                float rssiBaseAtual = minRssiBase + baseIndex;
                for (int nIndex = 0; nIndex < numN; ++nIndex) {
                    float N = Ns[nIndex];
                    for (int i = 0; i < numeroAmostras; ++i) {
                        float distanciaCalculada = pow(10, ((rssiBaseAtual * -1) - rssis[i]) / (10.0 * N));
                        desvios[baseIndex][nIndex] += abs(distanciaCalculada - distanciaAtual);
                    }
                }
            }

            Serial.println();
            Serial.printf("Distância atual %.1f metros\n", distanciaAtual);
            
            // Incrementa a distância atual
            distanciaAtual += 1;
            
            // Se ainda não tiver chegado na distância máxima, espera o reposicionamento
            if (distanciaAtual <= maxDistancia) {
                esperarPosicionamento();
            } else {
                // Finaliza mostrando os desvios
                float menorDesvio = INT_MAX / 1.0;
                int melhorBaseIndex = -1;
                int melhorNIndex = -1;
                
                for (int baseIndex = 0; baseIndex < numRssiBases; ++baseIndex) {
                    for (int nIndex = 0; nIndex < numN; ++nIndex) {
                        if (desvios[baseIndex][nIndex] < menorDesvio) {
                            menorDesvio = desvios[baseIndex][nIndex];
                            melhorBaseIndex = baseIndex;
                            melhorNIndex = nIndex;
                        }
                    }
                }

                Serial.println();
                Serial.printf("Melhor combinação: RSSI base = %d, N = %.1f, Desvio total = %.2f\n",
                              minRssiBase + melhorBaseIndex, Ns[melhorNIndex], menorDesvio);
                
                // Limpa variáveis
                rssis.clear();
                memset(desvios, 0, sizeof(desvios));
                distanciaAtual = 1;
                
                Serial.println("Aperte qualquer tecla para rodar novamente...");
                while (!Serial.available()) {
                    delay(100);
                }
                Serial.readString();
            }
            
            // Limpa as amostras para a próxima distância
            rssis.clear();
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

void setupScan() {
    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new AparelhosEscaneadosCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
}