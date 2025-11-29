#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>

#define LED_VERDE 7
#define LED_VERMELHO 6

const char* ssid = "iPhone de Maria Luiza";
const char* password = "malu1234";

const char* mqtt_server = "172.20.10.5";  
const char* mqtt_topic = "smartestoque/nivel";

WiFiClient espClient;
PubSubClient client(espClient);

// Callback → executa sempre que o sensor enviar dados
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("\nMensagem recebida do tópico ");
  Serial.println(topic);

  String payload = "";
  for (int i = 0; i < length; i++) payload += (char)message[i];

  Serial.println("Payload bruto: " + payload);

  // ------------- VARIÁVEIS ORIGINAIS (O QUE CHEGA) -----------
  float nivel = 0.0;
  String estado = "";

  int posNivel = payload.indexOf("nivel");
  int posEstado = payload.indexOf("estado");

  if (posNivel != -1) {
    int start = payload.indexOf(":", posNivel) + 1;
    int end = payload.indexOf(",", start);
    if (end == -1) end = payload.length();
    nivel = payload.substring(start, end).toFloat();
  }

  if (posEstado != -1) {
  int posDoisPontos = payload.indexOf(":", posEstado);
  if (posDoisPontos != -1) {
    int start = payload.indexOf("\"", posDoisPontos);
    int end = payload.indexOf("\"", start + 1);
    if (start != -1 && end != -1) {
      estado = payload.substring(start + 1, end); 
    }
  }
}

  //  ----------- MAPEAMENTO PARA NOMES DO BACKEND -----------
  float capacity_pct = nivel;

  bool below_limit = (estado == "CRITICO");

  //  ----------- LOG AMIGÁVEL NO SERIAL -----------
  Serial.println("-----------------------------------");
  Serial.print("nivel recebido: ");
  Serial.print(nivel);
  Serial.println("%");

  Serial.print("estado recebido: ");
  Serial.println(estado);

  Serial.print("capacity_pct (usado no backend): ");
  Serial.print(capacity_pct);
  Serial.println("%");

  Serial.print("below_limit (usado no backend): ");
  Serial.println(below_limit ? "TRUE (ALERTA)" : "FALSE (NORMAL)");

  if (below_limit) {
    Serial.println("⚠ ALERTA DO SENSOR → Estoque em nível crítico!");
  } else {
    Serial.println("✔ ESTADO NORMAL → Estoque em nível seguro.");
  }

  enviaParaBackend(capacity_pct, below_limit);

  Serial.println("-----------------------------------");
}

void conectaWiFi() {
  Serial.print("Conectando ao WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void conectaMQTT() {
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT...");
    Serial.print("WiFi status: ");
Serial.println(WiFi.status());   

Serial.print("MQTT broker: ");
Serial.println(mqtt_server);
    if (client.connect("gateway_estoque")) {
      Serial.println("Conectado ao Broker!");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("Falha, rc=");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void enviaParaBackend(float capacity_pct, bool below_limit) {
  HTTPClient http;

  http.begin("http://172.20.10.5:8000/telemetry"); 
  http.addHeader("Content-Type", "application/json");

  String jsonPayload = "{";
  jsonPayload += "\"capacity_pct\": " + String(capacity_pct) + ",";
  jsonPayload += "\"below_limit\": " + String(below_limit ? "true" : "false");
  jsonPayload += "}";

  int httpResponseCode = http.POST(jsonPayload);

  Serial.print("Enviando para backend: ");
  Serial.println(jsonPayload);

  Serial.print("Código HTTP: ");
  Serial.println(httpResponseCode);

  if (httpResponseCode >= 200 && httpResponseCode < 300) {
  digitalWrite(LED_VERDE, HIGH);
  digitalWrite(LED_VERMELHO, LOW);
} else {
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_VERMELHO, HIGH);
}

  http.end();
}


void setup() {
  Serial.begin(115200);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);

  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_VERMELHO, HIGH);

  conectaWiFi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) conectaMQTT();
  client.loop();
  delay(2000);
}
