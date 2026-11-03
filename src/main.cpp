#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

/*********
  Rui Santos
  Complete project details at:
  https://RandomNerdTutorials.com/esp32-hc-sr04-ultrasonic-arduino/
*********/

// Pinos do sensor
const int trigPin = 5;
const int echoPin = 18;

// Pino do rele
const int relePin = 19;

// Velocidade do som
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

// Distancias para controlar a lampada
#define DISTANCIA_LIGAR 15
#define DISTANCIA_DESLIGAR 17

long duration;
float distanceCm;
float distanceInch;

// Estado da lampada
bool lampadaLigada = false;

// Wi-Fi da ESP32
const char* ssid = "AMR_Esp32";
const char* password = "triodinamico";

// Configuracao do servidor
WebServer server(80);

// Configuracao do rele
#define RELE_LIGADO LOW
#define RELE_DESLIGADO HIGH

// Pagina HTML
const char pagina[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Automacao ESP32</title>
</head>

<body>

  <h1>Automacao ESP32</h1>

  <h2>Distancia atual:</h2>
  <p><span id="distancia">0</span> cm</p>

  <h2>Estado da lampada:</h2>
  <p id="lampada">Desligada</p>

  <script>

    function atualizarDados() {

      fetch('/dados')
      .then(response => response.json())
      .then(data => {

        document.getElementById("distancia").innerHTML =
          data.distancia;

        document.getElementById("lampada").innerHTML =
          data.lampada;

      });

    }

    setInterval(atualizarDados, 1000);

    atualizarDados();

  </script>

</body>
</html>
)rawliteral";

// Funcao da pagina principal
void handleRoot() {
  server.send_P(200, "text/html", pagina);
}

// Funcao para enviar os dados
void handleDados() {

  String estado;

  if (lampadaLigada == true) {
    estado = "Ligada";
  } else {
    estado = "Desligada";
  }

  String json = "{";
  json += "\"distancia\":" + String(distanceCm, 2) + ",";
  json += "\"lampada\":\"" + estado + "\"";
  json += "}";

  server.send(200, "application/json", json);
}

void setup() {

  Serial.begin(115200);

  // Configuracao dos pinos
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(relePin, OUTPUT);

  digitalWrite(relePin, RELE_DESLIGADO);

  // Inicia o Wi-Fi
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  Serial.println("Wi-Fi iniciado!");
  Serial.print("IP da ESP32: ");
  Serial.println(WiFi.softAPIP());

  // Configuracao das paginas
  server.on("/", handleRoot);
  server.on("/dados", handleDados);

  // Inicia o servidor
  server.begin();

  Serial.println("Servidor Web iniciado!");
}

void loop() {

  // Atende os clientes do servidor
  server.handleClient();

  // Limpa o trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Envia sinal ultrassonico
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Le o tempo do echoPin
  duration = pulseIn(echoPin, HIGH, 30000);

  // Calcula a distancia
  if (duration == 0) {
    distanceCm = -1;
  } else {
    distanceCm = duration * SOUND_SPEED / 2;
  }

  // Converte para polegadas
  distanceInch = distanceCm * CM_TO_INCH;

  // Controle da lampada
  if (distanceCm >= 0) {

    if (lampadaLigada == false &&
        distanceCm <= DISTANCIA_LIGAR) {

      lampadaLigada = true;
      digitalWrite(relePin, RELE_LIGADO);

    }

    else if (lampadaLigada == true &&
             distanceCm >= DISTANCIA_DESLIGAR) {

      lampadaLigada = false;
      digitalWrite(relePin, RELE_DESLIGADO);

    }

  }

  // Monitor Serial
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);

  Serial.print("Distance (inch): ");
  Serial.println(distanceInch);

  Serial.print("Lampada: ");

  if (lampadaLigada == true) {
    Serial.println("Ligada");
  } else {
    Serial.println("Desligada");
  }

  Serial.println("--------------------");

  delay(250);
}

