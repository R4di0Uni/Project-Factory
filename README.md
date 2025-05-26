# Project-Factory

## Carro Telecomandado via MQTT

Este projeto consiste em um carro controlado remotamente via comandos MQTT, com interface web para envio de comandos, backend em Node.js para comunicação com o broker MQTT, e um ESP32 responsável por receber e executar os comandos de movimento.

## Estrutura do Projeto

server.js: servidor Node.js que recebe comandos da interface web e os publica no broker MQTT.

docker-compose.yml: configura e inicia o broker MQTT (Eclipse Mosquitto) via Docker.

veiculo.ino: código embarcado que roda no ESP32, conecta-se ao broker MQTT, e controla os motores conforme os comandos recebidos.

Interface web (HTML/JS): envia comandos para o servidor via requisições HTTP.

Requisitos
Node.js e npm instalados

Docker e Docker Compose instalados

Extensão "Live Server" para VSCode (ou equivalente)

Placa ESP32

Rede local comum entre ESP32, computador e broker MQTT

Instalação e Execução
1. Clonar o repositório

git clone <url-do-repositorio>
cd <nome-do-diretorio>

2. Instalar dependências do servidor:

npm install express cors mqtt

3. Iniciar o broker MQTT com Docker:

docker-compose up -d
Certifique-se de que as pastas config, data e log estejam corretamente configuradas ou existam na raiz do projeto, mesmo que vazias.

4. Rodar o servidor Node.js

node server.js
O servidor ficará disponível em http://localhost:3000.

5. Iniciar a interface web
Abra o arquivo HTML principal da interface (por exemplo index.html) utilizando Live Server ou similar.

6. Carregar o código no ESP32

Conecte o ESP32 ao computador.

Altere as seguintes variáveis no arquivo veiculo.ino:

ssid: nome da rede Wi-Fi

password: senha da rede

mqtt_server: IP da máquina onde o broker MQTT está rodando (mesmo IP usado em server.js)

Faça o upload para o ESP32 utilizando o Arduino IDE.

## Observações
Todos os dispositivos devem estar conectados à mesma rede local.

Em caso de erro de conexão no ESP32, verifique se o broker está ativo e se o IP foi corretamente definido.

O tópico utilizado para envio de comandos MQTT é: esp32/car/move

Comandos válidos: up, down, left, right, stop

