const express = require("express");
const cors = require("cors");
const mqtt = require("mqtt");
const { Sequelize, DataTypes } = require("sequelize");

const app = express();
const PORT = 3000;
const distanceData = [];

app.use(cors());
app.use(express.json());

// Conexão com MQTT Broker
const brokerUrl = "mqtt://192.168.234.177";
const mqttClient = mqtt.connect(brokerUrl);

mqttClient.on("connect", () => {
  console.log("Connected to MQTT broker");
  mqttClient.subscribe("esp32/ultrasonic");
});

// Conexão com MySQL (ajuste host conforme necessário)
const sequelize = new Sequelize("carros", "user", "userpass", {
  host: "127.0.0.1",
  port: 3307,
  dialect: "mysql",
  logging: false, // <--- Desativa os logs SQL
});



// Definição do modelo
const Distancia = sequelize.define("Distancia", {
  distance: {
    type: DataTypes.FLOAT,
    allowNull: false,
  },
  timestamp: {
    type: DataTypes.DATE,
    allowNull: false,
  },
}, {
  tableName: "distancias",
  timestamps: false,
});

// Testar conexão e sincronizar tabela
(async () => {
  try {
    await sequelize.authenticate();
    console.log("Conexão com MySQL estabelecida.");
    await Distancia.sync(); // Cria a tabela se não existir
  } catch (err) {
    console.error("Erro ao conectar ao MySQL:", err);
  }
})();

// Recebe dados do sensor via MQTT
mqttClient.on("message", async (topic, message) => {
  if (topic === "esp32/ultrasonic") {
    const distance = parseFloat(message.toString());
    const timestamp = new Date();

    distanceData.push({ timestamp, distance });

    try {
      await Distancia.create({ distance, timestamp });
      console.log(`Distância recebida e salva: ${distance} cm`);
    } catch (err) {
      console.error("Erro ao salvar no MySQL:", err);
    }
  }
});

// API para enviar comandos ao ESP32
app.post("/move", (req, res) => {
  const { direction } = req.body;

  if (!["up", "down", "left", "right", "stop"].includes(direction)) {
    return res.status(400).json({ error: "Invalid direction" });
  }

  mqttClient.publish("esp32/car/move", direction, () => {
    console.log(`Command sent: ${direction}`);
    res.json({ message: `Command '${direction}' sent` });
  });
});


app.post('/speed', (req, res) => {
  const { speed } = req.body;
  const constrainedSpeed = Math.min(Math.max(speed, 0), 255);
  const message = `speed:${constrainedSpeed}`;
  mqttClient.publish('esp32/car/move', message); // <-- PUBLICA no MQTT com formato correto
  res.json({ message: `Velocidade enviada: ${constrainedSpeed}` });
});



// API para buscar histórico de distâncias
app.get("/distancias", async (req, res) => {
  try {
    const dados = await Distancia.findAll({
      order: [["timestamp", "DESC"]],
      limit: 10,
    });
    res.json(dados);
  } catch (err) {
    res.status(500).json({ error: "Erro ao buscar dados" });
  }
});

app.listen(PORT, () => {
  console.log(`Server running on http://localhost:${PORT}`);
});
