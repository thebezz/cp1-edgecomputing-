#include <Arduino.h> // necessario no PlatformIO (nao e .ino)
#include "DHT.h"

#include <WiFi.h>
#include <PubSubClient.h>

// ---- Wi-Fi (rede simulada do Wokwi) ----
const char *SSID = "Wokwi-GUEST";
const char *SENHA = "";

// ---- MQTT ----
const char *BROKER = "broker.hivemq.com";
const int PORTA = 1883;
const char *TOPICO = "fiap/edge/aula5/joaovitor/temp";    // troque SEUNOME
const char *TOPICO_LED = "fiap/edge/aula5/joaovitor/led"; // comando do LED

// ---- Sensor e atuadores ----
#define DHTPIN 15
#define DHTTYPE DHT22      // DHT 22 (AM2302)
#define BUZZER 23          // atuador sonoro
#define LED 21             // atuador luminoso
const float LIMIAR = 70.0; // % de umidade p/ o alarme
DHT dht(DHTPIN, DHTTYPE);

WiFiClient rede;
PubSubClient mqtt(rede);

void conectarWiFi()
{
  Serial.print("Conectando ao Wi-Fi");
  WiFi.begin(SSID, SENHA, 6); // canal 6 acelera no Wokwi
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado! IP: " + WiFi.localIP().toString());
}

// chamado a cada mensagem recebida
void callback(char *topico, byte *payload, unsigned int tamanho)
{
  String msg;
  for (unsigned int i = 0; i < tamanho; i++)
    msg += (char)payload[i];

  Serial.print("Recebido [");
  Serial.print(topico);
  Serial.print("]: ");
  Serial.println(msg);

  if (msg == "1" || msg == "on" || msg == "ON")
  {
    digitalWrite(LED, HIGH);
    Serial.println("LED ligado");
  }
  else if (msg == "0" || msg == "off" || msg == "OFF")
  {
    digitalWrite(LED, LOW);
    Serial.println("LED desligado");
  }
}

void conectarMQTT()
{
  mqtt.setServer(BROKER, PORTA);
  mqtt.setCallback(callback); // registra o callback
  while (!mqtt.connected())
  {
    Serial.print("Conectando ao broker MQTT...");
    String id = "esp32-fiap-" + String(random(0xffff), HEX);
    if (mqtt.connect(id.c_str()))
    {
      Serial.println(" conectado!");
      mqtt.subscribe(TOPICO_LED); // assina o topico do LED
      Serial.println("Inscrito em: " + String(TOPICO_LED));
    }
    else
    {
      Serial.print(" falhou, estado=");
      Serial.println(mqtt.state());
      delay(2000);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println(F("DHT22 - exemplo"));
  dht.begin();
  pinMode(BUZZER, OUTPUT);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW); // comeca apagado
  conectarWiFi();
  conectarMQTT();
}

void loop()
{
  if (!mqtt.connected())
    conectarMQTT();
  mqtt.loop();

  float temp = dht.readTemperature();
  float umid = dht.readHumidity();

  if (isnan(temp) || isnan(umid))
  {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // decisao local: umidade alta -> alarme sonoro
  if (umid > LIMIAR)
  {
    tone(BUZZER, 500);
    delay(1000);
    tone(BUZZER, 800);
    delay(1000);
  }
  else
  {
    noTone(BUZZER);
  }

  Serial.print(F("Umidade: "));
  Serial.print(umid);
  Serial.print(F("%  Temperatura: "));
  Serial.print(temp);
  Serial.println(F(" C"));

  // publica a temperatura (QoS 0)
  char msg[16];
  dtostrf(temp, 4, 1, msg);
  mqtt.publish(TOPICO, msg);

  Serial.print("Publicado em ");
  Serial.print(TOPICO);
  Serial.print(": ");
  Serial.println(msg);

  delay(2000);
}
