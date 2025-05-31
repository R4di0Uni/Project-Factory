const express = require("express");
const cors = require("cors");
const mqtt = require("mqtt");
const mongoose = require("mongoose");

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

// Conexão com MongoDB
mongoose.connect("mongodb://localhost:27017/carros", {
  
});

const distanciaSchema = new mongoose.Schema({
  distance: Number,
  timestamp: Date,
});
const Distancia = mongoose.model("Distancia", distanciaSchema);

// Recebe dados do sensor via MQTT
mqttClient.on("message", async (topic, message) => {
  if (topic === "esp32/ultrasonic") {
    const distance = parseFloat(message.toString());
    const timestamp = new Date();

    distanceData.push({ timestamp, distance });
    await Distancia.create({ distance, timestamp });

    console.log(`Distância recebida e salva: ${distance} cm`);
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

// API para buscar histórico de distâncias
app.get("/distancias", async (req, res) => {
  const dados = await Distancia.find().sort({ timestamp: -1 }).limit(10);
  res.json(dados);
});

app.listen(PORT, () => {
  console.log(`Server running on http://localhost:${PORT}`);
});
