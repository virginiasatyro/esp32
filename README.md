# esp32

## Install ESP-IDF

[ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html#get-started-get-esp-idf)

### Linux - Ubuntu

#### 1 - Setup Toolchain para Linux

- Ubuntu and Debian:

```sudo apt-get install git wget flex bison gperf python python-pip python-setuptools python-serial python-click python-cryptography python-future python-pyparsing python-pyelftools cmake ninja-build ccache libffi-dev libssl-dev```

```sudo apt-get install git wget libncurses-dev flex bison gperf python python-pip python-setuptools python-serial python-click python-cryptography python-future python-pyparsing python-pyelftools cmake ninja-build ccache libffi-dev libssl-dev```

#### 2 - ESP-IDF

```cd ~/esp```

```git clone --recursive https://github.com/espressif/esp-idf.git```

#### 3 - Configuração das Ferramentas

```cd ~/esp/esp-idf``` 
```./install.sh```

#### 4 - Configuração das Variáveis de Ambiente 

```. $HOME/esp/esp-idf/export.sh```

É necessário escrever essa linha de código nos diretórios onde os programas serão compilados!

#### 5 - Iniciar um Projeto

```cd ~/esp```

```cp -r $IDF_PATH/examples/get-started/hello_world .```

#### 6 - Conectar o Dispositivo 

```/dev/tty```

#### 7 - Configuração

```cd ~/esp/hello_world```

```idf.py menuconfig```

ou simplesmente:

```make menuconfig```

#### 8 - Compilar projeto 

```idf.py build```

ou simplesmente:

```make all``` - compila tudo 

```make v=1``` - compila tudo novamente, apagando configurações anteriores

#### 9 - Gravação 

```idf.py -p PORT [-b BAUD] flash```

ou simplesmente:

```make flash```

#### 10 - Monitor 

```idf.py -p PORT monitor```

ou simplesmente:

```make monitor```

para sair utilize: ```Ctrl + ]```

podemos utilizar também:

```make flash monitor```
