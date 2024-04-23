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
