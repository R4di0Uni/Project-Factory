const express = require('express');
const mqtt = require('mqtt');

const app = express();
const port = 3000;

// Configuração do broker MQTT
const brokerUrl = 'mqtt://broker.hivemq.com'; 
const client = mqtt.connect(brokerUrl);

// Conectar ao broker MQTT
client.on('connect', () => {
    console.log('Conectado ao broker MQTT');

    
    client.subscribe('esp32/dados', (err) => {
        if (!err) {
            console.log('Inscrito no tópico: esp32/dados');
        }
    });
});

client.on('message', (topic, message) => {
    console.log(`Mensagem recebida do ESP32: ${message.toString()}`);
});

// Rota principal
app.get('/', (req, res) => {
    res.send('WebServer do Nosso Veículo!');
});


app.get('/comando/:msg', (req, res) => {
    const msg = req.params.msg;
    client.publish('esp32/comando', msg); 
    res.send(`Comando enviado: ${msg}`);
});

// Inicia o servidor
app.listen(port, () => {
    console.log(`Servidor rodando em http://localhost:${port}`);
});
