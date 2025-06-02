#include <WiFi.h>
#include <PubSubClient.h>

#define PWM_FREQ 5000
#define PWM_RES 8  // 8-bit resolution (0–255)

// WiFi e MQTT
const char* ssid = "Redmi 14C";
const char* password = "onepiece";
const char* mqtt_server = "192.168.234.177";

WiFiClient espClient;
PubSubClient client(espClient);

// Pinos do motor
const int motorLeftA = 5;
const int motorLeftB = 18;
const int motorRightA = 19;
const int motorRightB = 21;
const int enaPin = 23;
const int enbPin = 22;

// Sensor ultrassônico frontal
const int trigPin = 27;
const int echoPin = 14;

int speedValue = 255;

// Controle de navegação automática
bool autoMode = false;
bool avoiding = false;
String avoidingDirection = "";
unsigned long lastAutoNavTime = 0;
const unsigned long autoNavInterval = 100;

// Publicação de distância
unsigned long lastPublishTime = 0;
const unsigned long publishInterval = 2000;

void setup() {
  Serial.begin(115200);

  // WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  // MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32CarClient123")) {
      Serial.println("connected to MQTT broker!");
      client.subscribe("esp32/car/move");
    } else {
      Serial.print("failed, rc=");
      Serial.println(client.state());
      delay(1000);
    }
  }

  // Pinos
  pinMode(motorLeftA, OUTPUT);
  pinMode(motorLeftB, OUTPUT);
  pinMode(motorRightA, OUTPUT);
  pinMode(motorRightB, OUTPUT);
  pinMode(enaPin, OUTPUT);
  pinMode(enbPin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // PWM
  ledcAttach(enaPin, PWM_FREQ, PWM_RES);
  ledcAttach(enbPin, PWM_FREQ, PWM_RES);
  ledcWrite(enaPin, speedValue);
  ledcWrite(enbPin, speedValue);
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }

  client.loop();

  // Enviar distância a cada 2s
  unsigned long now = millis();
  if (now - lastPublishTime >= publishInterval) {
    lastPublishTime = now;
    float distance = getDistance(trigPin, echoPin);
    char msg[10];
    dtostrf(distance, 1, 2, msg);
    client.publish("esp32/ultrasonic", msg);
  }

  // Navegação autônoma
  if (autoMode && millis() - lastAutoNavTime > autoNavInterval) {
    lastAutoNavTime = millis();
    autoNavigate();
  }
}

// ----------- MQTT -----------
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32CarClient123")) {
      Serial.println("MQTT reconnected");
      client.subscribe("esp32/car/move");
    } else {
      Serial.print("MQTT connection failed, rc=");
      Serial.println(client.state());
      delay(1000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("Callback triggered");
  String command = "";
  for (unsigned int i = 0; i < length; i++) {
    command += (char)payload[i];
  }
  Serial.println("Command Received: " + command);

  if (command == "up") moveForward();
  else if (command == "down") moveBackward();
  else if (command == "left") turnLeft();
  else if (command == "right") turnRight();
  else if (command == "stop") stopMotors();
  else if (command.startsWith("speed:")) {
    speedValue = command.substring(6).toInt();
    Serial.print("Setting speed to: ");
    Serial.println(speedValue);
    ledcWrite(enaPin, speedValue);
    ledcWrite(enbPin, speedValue);
  }
  else if (command == "auto_on") {
    autoMode = true;
    Serial.println("AUTO mode ON");
  } else if (command == "auto_off") {
    autoMode = false;
    avoiding = false;
    avoidingDirection = "";
    stopMotors();
    Serial.println("AUTO mode OFF");
  }
}

// ----------- Motor Controls -----------
void moveForward() {
  digitalWrite(motorLeftA, LOW); digitalWrite(motorLeftB, HIGH);
  digitalWrite(motorRightA, LOW); digitalWrite(motorRightB, HIGH);
}

void moveBackward() {
  digitalWrite(motorLeftA, HIGH); digitalWrite(motorLeftB, LOW);
  digitalWrite(motorRightA, HIGH); digitalWrite(motorRightB, LOW);
}

void turnLeft() {
  digitalWrite(motorLeftA, LOW); digitalWrite(motorLeftB, HIGH);
  digitalWrite(motorRightA, HIGH); digitalWrite(motorRightB, LOW);
}

void turnRight() {
  digitalWrite(motorLeftA, HIGH); digitalWrite(motorLeftB, LOW);
  digitalWrite(motorRightA, LOW); digitalWrite(motorRightB, HIGH);
}

void stopMotors() {
  digitalWrite(motorLeftA, LOW);
  digitalWrite(motorLeftB, LOW);
  digitalWrite(motorRightA, LOW);
  digitalWrite(motorRightB, LOW);
}

// ----------- Ultrassônico -----------
float getDistance(int trig, int echo) {
  digitalWrite(trig, LOW); delayMicroseconds(2);
  digitalWrite(trig, HIGH); delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long duration = pulseIn(echo, HIGH, 30000); // 30ms timeout
  return duration * 0.034 / 2.0;
}

// ----------- Navegação Autônoma -----------
void autoNavigate() {
  float distance = getDistance(trigPin, echoPin);
  Serial.print("Auto | Distância: "); Serial.println(distance);

  if (avoiding) {
    if (avoidingDirection == "right") {
      turnRight();
      if (distance > 25) {
        avoiding = false;
        avoidingDirection = "";
        moveForward();
        delay(300);
      }
    } else if (avoidingDirection == "left") {
      turnLeft();
      if (distance > 25) {
        avoiding = false;
        avoidingDirection = "";
        moveForward();
        delay(300);
      }
    }
    delay(200);
    return;
  }

  if (distance > 25) {
    moveForward();
  } else {
    Serial.println("Obstacle detected — stopping and turning");
    avoiding = true;
    // Alternar direção de desvio se desejar
    avoidingDirection = (millis() % 2 == 0) ? "right" : "left";
    stopMotors();
    delay(100);
  }
}
