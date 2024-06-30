# projeto-redes
Projeto redes UDESC

O desafio do projeto consiste em desenvolver um localizador de objetos em uma distância curta de
10 metros usando Bluetooth Low Energy com monitoramento IoT e com o protocolo MQTT.
Deverá ser desenvolvida uma aplicação que permita a localização baseada na proximidade de um
objeto dentro do raio do Raio Signal Strenght Indicator (RSSI) gerado pelo campo de alcance do
Bluetooth Low Energy fornecido pelo microcontrolador ESP32.
Como objetos podem ser utilizados por exemplo, o Bluetooth Low Energy (BLE) presente nas
pulseiras inteligentes, relógios inteligentes e nos celulares, pois apresentam o recurso que detecta o
sinal através do cálculo da distância em que o ESP32 se encontra, gerando um raio e a localização
do objeto nesse raio.
A partir do desenvolvimento do localizador que utiliza a comunicação Bluetooth Low Energy
(BLE) para obter a distância do raio de acesso, mensagens devem ser enviadas por meio de IoT,
usando-se o protocolo MQTT (Message Queuing Telemetry Transport) com o intuito de transmitir
as informações sobre o número de objetos encontrados e a distância entre eles. 



## Arquivos do Projeto

### scanner.ino

Este arquivo contém o código para:

- Conectar o ESP32 a uma rede Wi-Fi.
- Estabelecer conexão com um broker MQTT.
- Realizar varreduras BLE contínuas, buscando dispositivos específicos.
- Calcular a distância dos dispositivos com base no RSSI e enviar esses dados via MQTT.

### calibrador.ino

Este arquivo é usado para calibrar os parâmetros `rssiBase` e `N`, que são críticos para a precisão na estimativa de distâncias dos dispositivos BLE. O calibrador realiza varreduras para coletar dados de RSSI em diferentes distâncias predefinidas e ajusta os parâmetros para minimizar o desvio das estimativas de distância.

## Configuração e Uso

### Pré-requisitos

- Hardware:
    - ESP32
    - Computador com Arduino IDE instalado
- Software:
    - Bibliotecas do Arduino:
        - `BLEDevice.h`
        - `ArduinoMqttClient.h`
        - `WiFi.h`
        - `algorithm`

### Configuração

1. Carregue os códigos `scanner.ino` e `calibrador.ino` no seu ESP32 usando a Arduino IDE.
2. Certifique-se de que todas as bibliotecas necessárias estão instaladas através do Gerenciador de Bibliotecas do Arduino.

### Operação

1. **scanner.ino**:
    - Modifique as variáveis `nomeRede` e `senhaRede` para corresponder às suas configurações de Wi-Fi.
    - Ajuste `brokerIp` e `brokerPort` para apontar para o seu broker MQTT.
    - Inicie o ESP32 e observe a saída no monitor serial para verificar a conexão com a rede e o broker.
    - A distância estimada dos dispositivos será enviada automaticamente para o tópico MQTT configurado.
2. **calibrador.ino**:
    - Execute este código para determinar os melhores valores de `rssiBase` e `N` antes de utilizar o `scanner.ino` para garantir a precisão.
    - Siga as instruções no monitor serial para posicionar o dispositivo a diferentes distâncias.
