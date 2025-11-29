#include <WiFi.h>
#include <PubSubClient.h>

// ----------- PINOS -----------
#define TRIG_PIN 13
#define ECHO_PIN 12
#define LED_VERDE 6
#define LED_VERMELHO 7
#define BUZZER_PIN 18  
#define BOTAO_PIN 4    

// ----------- CONFIGURAÇÕES DO RECIPIENTE -----------
const float ALTURA_RECIPIENTE = 15.0;  
const float LIMITE_CRITICO = 50.0;     

// ----------- WiFi / MQTT -----------
const char* ssid = "iPhone de Maria Luiza";
const char* password = "malu1234";

const char* mqtt_server = "172.20.10.5";  
const char* mqtt_topic = "smartestoque/nivel";

WiFiClient espClient;
PubSubClient client(espClient);

// ----------- VARIÁVEIS DO SISTEMA -----------
volatile bool sistemaAtivo = true;  
unsigned long debounceTempo = 0;

// ------------ INTERRUPÇÃO ------------
void IRAM_ATTR botaoInterrupt() {
  unsigned long agora = millis();

  // Debounce de interrupção
  if (agora - debounceTempo > 300) {
    sistemaAtivo = !sistemaAtivo;
    debounceTempo = agora;
  }
}

// ------------ FUNÇÕES ------------
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
    if (client.connect("nó_sensor_ultrassom")) {
      Serial.println("Conectado ao Broker MQTT!");
    } else {
      Serial.print("Falha, rc=");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

float medirDistancia() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duracao = pulseIn(ECHO_PIN, HIGH);
  float distancia = duracao * 0.034 / 2;

  return distancia;  // em cm
}

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Botão com pull-up interno
  pinMode(BOTAO_PIN, INPUT_PULLUP);

  // Interrupção no botão
  attachInterrupt(digitalPinToInterrupt(BOTAO_PIN), botaoInterrupt, FALLING);

  conectaWiFi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) conectaMQTT();
  client.loop();

  // ------------ SE O SISTEMA ESTÁ DESLIGADO ------------
  if (!sistemaAtivo) {
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_VERMELHO, LOW);
    digitalWrite(BUZZER_PIN, LOW);

    Serial.println("\n❌ SISTEMA DESLIGADO - Aguardando botão...");
    delay(500);
    return;  
  }

  // ------------ SISTEMA LIGADO - LÊ SENSOR ------------
  float distancia = medirDistancia();
  float nivel = ALTURA_RECIPIENTE - distancia;  

  if (nivel < 0) nivel = 0;
  if (nivel > ALTURA_RECIPIENTE) nivel = ALTURA_RECIPIENTE;

  float porcentagem = (nivel / ALTURA_RECIPIENTE) * 100.0;

  // ------------ EXIBIR MEDIÇÕES NO SERIAL ------------
  Serial.println("\n-----------------------------");
  Serial.print("Distância medida: ");
  Serial.print(distancia);
  Serial.println(" cm");

  Serial.print("Nível calculado: ");
  Serial.print(nivel);
  Serial.println(" cm");

  Serial.print("Porcentagem restante: ");
  Serial.print(porcentagem);
  Serial.println(" %");

  // ------------ LEDs + BUZZER ------------
  if (porcentagem <= LIMITE_CRITICO) {
    digitalWrite(LED_VERMELHO, HIGH);
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(BUZZER_PIN, HIGH);
    Serial.println("⚠ ALERTA: Nível crítico! Estoque está acabando!");


  } else {
    digitalWrite(LED_VERDE, HIGH);
    digitalWrite(LED_VERMELHO, LOW);
    digitalWrite(BUZZER_PIN, LOW);

    Serial.println("✔ ESTADO NORMAL: Nível dentro da faixa segura.");

    digitalWrite(BUZZER_PIN, LOW);
  }

  // ------------ Enviando via MQTT ------------
  String payload = String("{\"nivel\":") + porcentagem + ",\"estado\":\"" 
                   + (porcentagem <= LIMITE_CRITICO ? "CRITICO" : "OK") + "\"}";
  
  client.publish(mqtt_topic, payload.c_str());

  Serial.print("Enviado ao MQTT: ");
  Serial.println(payload);

  delay(2000);
}
