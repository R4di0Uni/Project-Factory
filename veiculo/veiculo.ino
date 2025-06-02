  #include <WiFi.h>
  #include <PubSubClient.h>
  #define PWM_FREQ 5000
  #define PWM_RES 8  // 8-bit resolution (0–255)


  // Configurações WiFi e MQTT
  const char* ssid = "Redmi 14C";
  const char* password = "onepiece";
  const char* mqtt_server = "192.168.234.177";

  WiFiClient espClient;
  PubSubClient client(espClient);

  const int motorLeftA = 5;
  const int motorLeftB = 18;
  const int motorRightA = 19;
  const int motorRightB = 21;

  const int enaPin = 23;
  const int enbPin = 22;

  const int trigPin = 27;
  const int echoPin = 14;

int speedValue = 255;  // Default speed value



  void moveForward();
  void moveBackward();
  void turnLeft();
  void turnRight();
  void stopMotors();

  void callback(char* topic, byte* payload, unsigned int length);

  void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32CarClient123")) {
      Serial.println("connected to MQTT broker!");
      if (client.subscribe("esp32/car/move")) {
        Serial.println("Subscribed to esp32/car/move");
      }
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 1 second");
      delay(1000);
    }
  }

  pinMode(motorLeftA, OUTPUT);
  pinMode(motorLeftB, OUTPUT);
  pinMode(motorRightA, OUTPUT);
  pinMode(motorRightB, OUTPUT);
  pinMode(enaPin, OUTPUT);
  pinMode(enbPin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  
  ledcAttach(enaPin, PWM_FREQ, PWM_RES);
  ledcAttach(enbPin, PWM_FREQ, PWM_RES);

  ledcWrite(enaPin, speedValue);  
  ledcWrite(enbPin, speedValue);
 


  }


unsigned long lastPublishTime = 0;
const unsigned long publishInterval = 2000; // 2 segundos

void loop() {
  if (!client.connected()) {
    Serial.println("MQTT disconnected, trying to reconnect...");
    while (!client.connected()) {
      Serial.print("Attempting MQTT connection...");
      if (client.connect("ESP32CarClient123", NULL, NULL, NULL, 0, false, NULL, 120)) {
        Serial.println("MQTT reconnected");
        client.subscribe("esp32/car/move");
      } else {
        Serial.print("MQTT connection failed, rc=");
        Serial.println(client.state());
        delay(1000);
      }
    }
  }

  client.loop(); // Deve rodar o tempo todo para receber comandos

  unsigned long now = millis();
  if (now - lastPublishTime >= publishInterval) {
  lastPublishTime = now;

  digitalWrite(trigPin, LOW); delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000); // 30ms timeout
  float distance = duration * 0.034 / 2.0;

  char msg[10];
  dtostrf(distance, 1, 2, msg);
  client.publish("esp32/ultrasonic", msg);
}

}




  void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("Callback triggered");
  String command = "";
  for (unsigned int i = 0; i < length; i++) {
    command += (char)payload[i];
  }
  Serial.println("Command Received: " + command);

  if (command == "up") {
    moveForward();
  } else if (command == "down") {
    moveBackward();
  } else if (command == "left") {
    turnLeft();
  } else if (command == "right") {
    turnRight();
  } else if (command == "stop") {
    stopMotors();
    } else if (command.startsWith("speed:")) {
    int speedValue = command.substring(6).toInt();
    Serial.print("Setting motor speed to: ");
    Serial.println(speedValue);
    ledcWrite(enaPin, speedValue);
    ledcWrite(enbPin, speedValue);

  }
  
}




  void moveForward() {
    digitalWrite(motorLeftA, LOW);    // Esquerdo para trás
    digitalWrite(motorLeftB, HIGH);
    digitalWrite(motorRightA, LOW);   // Direito para trás
    digitalWrite(motorRightB, HIGH);
  }

  void moveBackward() {
    digitalWrite(motorLeftA, HIGH);   // Esquerdo para frente
    digitalWrite(motorLeftB, LOW);
    digitalWrite(motorRightA, HIGH); // Direito para frente
    digitalWrite(motorRightB, LOW);
    
  }

  void turnLeft() {
    digitalWrite(motorLeftA, LOW);    // Esquerdo para trás
    digitalWrite(motorLeftB, HIGH);
    digitalWrite(motorRightA, HIGH);  // Direito para frente
    digitalWrite(motorRightB, LOW);
  }

  void turnRight() {
    digitalWrite(motorLeftA, HIGH);   // Esquerdo para frente
    digitalWrite(motorLeftB, LOW);
    digitalWrite(motorRightA, LOW);   // Direito para trás
    digitalWrite(motorRightB, HIGH);
  }



  void stopMotors() {
    digitalWrite(motorLeftA, LOW);
    digitalWrite(motorLeftB, LOW);
    digitalWrite(motorRightA, LOW);
    digitalWrite(motorRightB, LOW);
  }
