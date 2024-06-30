# projeto-redes
Projeto redes UDESC

Este projeto é uma solução integrada que utiliza um microcontrolador ESP32 para varrer dispositivos Bluetooth Low Energy (BLE) e comunicar as informações obtidas através de um broker MQTT. Ele é composto por dois arquivos principais: `scanner.ino` para a operação de varredura e comunicação, e `calibrador.ino` para calibração dos parâmetros de distância. O objetivo é primeiro utilizar o `calibrador.ino` para determinar os melhores valores de `N` e `rssiBase`, que são essenciais para a precisão na estimativa de distância dos dispositivos BLE. Após a calibração, esses valores devem ser inseridos no código `scanner.ino` para realizar a varredura efetiva e comunicação dos dados.

## Arquivos do Projeto

### scanner.ino

Responsável por:

- Conectar o ESP32 a uma rede Wi-Fi.
- Estabelecer uma conexão com um broker MQTT.
- Realizar varreduras BLE contínuas de dispositivos específicos.
- Calcular a distância baseada no RSSI e enviar esses dados via MQTT usando os parâmetros calibrados.

### calibrador.ino

Utilizado para ajustar os parâmetros `rssiBase` e `N`. Este arquivo realiza varreduras para coletar dados de RSSI em diferentes distâncias predefinidas, ajustando os parâmetros para minimizar o desvio das estimativas de distância. Os resultados desta calibração são essenciais para a configuração do arquivo `scanner.ino`.

## Configuração e Uso

### Pré-requisitos

- Hardware:
    - ESP32 Dev Module
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

- **calibrador.ino**:
    - Execute este código primeiro para determinar os valores ótimos de `rssiBase` e `N`.
    - Siga as instruções no monitor serial para calibrar corretamente os parâmetros.
    - Após a calibração, anote os valores obtidos para `rssiBase` e `N`.
- **scanner.ino**:
    - Com os valores de `rssiBase` e `N` obtidos, atualize as variáveis correspondentes no arquivo `scanner.ino`.
    - Modifique também as variáveis `nomeRede`, `senhaRede`, `brokerIp`, e `brokerPort` para corresponder às suas configurações de rede e MQTT.
    - Inicie o ESP32 e observe a saída no monitor serial para verificar a conexão com a rede e o broker.
    - As distâncias estimadas dos dispositivos BLE detectados serão enviadas ao tópico MQTT configurado.
