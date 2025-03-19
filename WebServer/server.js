const express = require("express");
const mqtt = require("mqtt");

const app = express();
const PORT = 3000;

// MQTT Broker Connection
const brokerUrl = "mqtt://test.mosquitto.org"; // Change to your broker URL if needed
const mqttClient = mqtt.connect(brokerUrl);

mqttClient.on("connect", () => {
  console.log("Connected to MQTT broker");
});

// Middleware for parsing JSON requests
app.use(express.json());

// MQTT topic for ESP32
const topic = "esp32/car/move";

// API Endpoint to send movement commands
app.post("/move", (req, res) => {
  const { direction } = req.body;

  if (!["up", "down", "left", "right"].includes(direction)) {
    return res.status(400).json({ error: "Invalid direction" });
  }

  mqttClient.publish(topic, direction, () => {
    console.log(`Command sent: ${direction}`);
    res.json({ message: `Command '${direction}' sent` });
  });
});

app.listen(PORT, () => {
  console.log(`Server running on http://localhost:${PORT}`);
});
