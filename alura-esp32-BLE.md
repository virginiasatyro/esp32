# Alura

## Esp32: Detector de presença com Bluetooth Low Energy

### Aula 1: Preparando o projeto

#### Introdução
#### Detalhando o projeto
- características
- tabela de pinos
- comparação com o ESP8266
- introdução ao bluetooth low energy
#### Instalando as bibliotecas
- instalação do ESP32 na IDE do Arduino
- https://github.com/espressif/arduino-esp32
- instalação do arduino no linux: http://autocorerobotica.blog.br/como-instalar-a-arduino-ide-no-ubuntu/
- pacotes do ESP32 para Arduino: https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-mac-and-linux-instructions/
#### Testando o ESP32
- teste para imprimir no serial um número aleatório


### Aula 2: Interagindo com o BLE
#### Comparando Bluetooths
Bluetooth Classic x Low Energy
Em comum
- 2.4GHz
- Tecnologia de pareamento, autenticação, criptografia
- Master/Slave Mode
#### BLE Server
- Download aplicativo: nRF Connect
- Exemplo Arduino: BLE_server
Diferenças
- LE -> baixo consumo - picos de 15-20mA
- Taxa de transferência: clássico: 2 a 3Mbps;LE: 200Kbps;
- Aplicações: clássic: streaming de dados, voz, áudio; LE: transferência de dados não contínua (manda um dado, dorme);
- exemplos, clássico: fone de ouvido, caixa de som; LE: mi band;
- https://learn.adafruit.com/introduction-to-bluetooth-low-energy/gatt
- mais informações sobre GAAT: http://dev.ti.com/tirex/content/simplelink_cc2640r2_sdk_1_40_00_45/docs/blestack/ble_user_guide/html/ble-stack-3.x/gatt.html
- serviços: https://www.bluetooth.com/specifications/gatt/services/
- características: https://www.bluetooth.com/specifications/gatt/characteristics/
- exemplo: BLE_server e BLE_write

### Aula 3: Detectando dispositivo

#### BLEScan
- programa para encontrar dispositivos: BLEScan -> detectorBLE
#### Smartphone BLE
- para detectar o Smartphone como uma dispositivo bloetooth LE: advertiser -> + -> adicionar Smartphone. Cria um MAC, faz scan do telefone.
#### Filtrando o Scan
- como encontrar determinado dispositivo e o RSSI ("distância" do dispositivo)

### Aula 4: Conectividade do ESP32

### Aula 5: Recursos extras

-----------
instalaão esp-ide https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html#step-1-install-prerequisites
------------
tutotias:
http://labdegaragem.com/profiles/blogs/tutorial-conhecendo-o-esp32-usando-esp-idf-4
--------------
