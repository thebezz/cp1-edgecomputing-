/*
 * FIAP - Edge Computing - Checkpoint 1
 * Nó de Borda Inteligente - Câmara-piloto FrioLog
 *
 * Sensores:
 *   - DHT22 (digital)  -> temperatura e umidade da câmara
 *   - LDR   (analógico) -> luminosidade interna (câmara fechada = escura;
 *                          luz = porta aberta ou mal vedada)
 * Atuadores:
 *   - Relé   -> aciona a refrigeração de reforço (LED azul "Compressor")
 *   - Buzzer -> alarme sonoro local, com padrão diferente para cada estado
 *
 * Toda a decisão acontece no ESP32. Não há Wi-Fi, MQTT nem HTTP neste código:
 * o nó funciona sozinho, mesmo sem internet.
 */

#include <Arduino.h>
#include <DHT.h>
#include <math.h>

// ------------------------- Pinos -------------------------
#define PINO_DHT 15
#define PINO_LDR 34 // ADC1 (entrada analógica)
#define PINO_RELE 26
#define PINO_BUZZER 25
#define TIPO_DHT DHT22

// ------------- Limites (câmara de medicamentos: 2 a 8 °C) -------------
const float TEMP_MIN_IDEAL = 2.0;     // abaixo disso: ATENCAO (risco de congelar)
const float TEMP_MAX_IDEAL = 8.0;     // acima disso:  ATENCAO
const float TEMP_MIN_CRITICA = 0.0;   // abaixo disso: CRITICO
const float TEMP_MAX_CRITICA = 10.0;  // acima disso:  CRITICO
const float UMID_MAX_IDEAL = 70.0;    // %
const float UMID_MAX_CRITICA = 85.0;  // %
const float LUX_PORTA_ABERTA = 100.0; // acima disso considera a porta aberta

// Histerese: para SAIR de um estado a leitura precisa voltar um pouco além
// do limite. Evita que o estado fique "piscando" perto do ponto de decisão.
const float HIST_TEMP = 0.5; // °C
const float HIST_UMID = 2.0; // %
const float HIST_LUX = 20.0; // lux

// Constantes do LDR do Wokwi (conversão tensão -> lux)
const float GAMMA = 0.7;
const float RL10 = 50.0;

const unsigned long INTERVALO_LEITURA = 2000; // DHT22 aceita no máx. 1 leitura a cada 2 s

// ------------------------- Estados -------------------------
enum Estado
{
  NORMAL,
  ATENCAO,
  CRITICO,
  FALHA_SENSOR
};

Estado estadoAtual = NORMAL; // variável que representa o estado do sistema
String motivoAtual = "inicializacao";
bool frioExcessivo = false; // se o problema é frio, o compressor NÃO deve ligar

DHT dht(PINO_DHT, TIPO_DHT);

unsigned long ultimaLeitura = 0;
int freqBuzzerAtual = 0; // 0 = buzzer desligado

// ---------------------------------------------------------------------------

const char *nomeEstado(Estado e)
{
  switch (e)
  {
  case NORMAL:
    return "NORMAL";
  case ATENCAO:
    return "ATENCAO";
  case CRITICO:
    return "CRITICO";
  case FALHA_SENSOR:
    return "FALHA_SENSOR";
  }
  return "?";
}

float lerLux()
{
  int bruto = analogRead(PINO_LDR);
  float tensao = bruto / 4095.0 * 3.3;
  if (tensao >= 3.29)
    return 0.0; // escuro total
  if (tensao <= 0.01)
    return 100000.0; // saturado de luz
  float resistencia = 2000.0 * tensao / (1.0 - tensao / 3.3);
  return pow(RL10 * 1e3 * pow(10, GAMMA) / resistencia, 1.0 / GAMMA);
}

/*
 * FUNÇÃO DE DECISÃO LOCAL
 * Recebe as leituras dos dois sensores e devolve o estado que o nó deve assumir.
 * Prioridade (da maior para a menor): FALHA_SENSOR > CRITICO > ATENCAO > NORMAL.
 * As regras são testadas nessa ordem e a primeira verdadeira vence, então,
 * se várias condições forem verdadeiras ao mesmo tempo, prevalece a mais grave.
 */
Estado decidirEstado(float temp, float umid, float lux, String &motivo, bool &frio)
{
  frio = false;

  // 1) Sem leitura confiável não dá para decidir com segurança
  if (isnan(temp) || isnan(umid))
  {
    motivo = "falha na leitura do DHT22";
    return FALHA_SENSOR;
  }

  // Histerese: se já estou num estado, fica um pouco mais "fácil" permanecer nele
  bool jaCritico = (estadoAtual == CRITICO);
  bool jaAlerta = (estadoAtual == CRITICO || estadoAtual == ATENCAO);

  float hTc = jaCritico ? HIST_TEMP : 0;
  float hUc = jaCritico ? HIST_UMID : 0;
  float hTa = jaAlerta ? HIST_TEMP : 0;
  float hUa = jaAlerta ? HIST_UMID : 0;
  float hL = jaAlerta ? HIST_LUX : 0;

  bool portaAberta = lux > (LUX_PORTA_ABERTA - hL);

  // 2) CRITICO
  if (portaAberta && temp > TEMP_MAX_IDEAL - hTc)
  { // regra combinada (2 sensores)
    motivo = "porta aberta com temperatura acima de 8 C";
    return CRITICO;
  }
  if (temp > TEMP_MAX_CRITICA - hTc)
  {
    motivo = "temperatura acima do limite critico";
    return CRITICO;
  }
  if (temp < TEMP_MIN_CRITICA + hTc)
  {
    motivo = "temperatura abaixo do limite critico (congelamento)";
    frio = true;
    return CRITICO;
  }
  if (umid > UMID_MAX_CRITICA - hUc)
  {
    motivo = "umidade acima do limite critico";
    return CRITICO;
  }

  // 3) ATENCAO
  if (temp > TEMP_MAX_IDEAL - hTa)
  {
    motivo = "temperatura acima da faixa ideal";
    return ATENCAO;
  }
  if (temp < TEMP_MIN_IDEAL + hTa)
  {
    motivo = "temperatura abaixo da faixa ideal";
    frio = true;
    return ATENCAO;
  }
  if (umid > UMID_MAX_IDEAL - hUa)
  {
    motivo = "umidade acima da faixa ideal";
    return ATENCAO;
  }
  if (portaAberta)
  {
    motivo = "porta aberta (luz detectada)";
    return ATENCAO;
  }

  // 4) Nenhuma regra disparou
  motivo = "tudo dentro da faixa";
  return NORMAL;
}

// Relé segue o estado decidido
void atualizarRele()
{
  bool ligar = (estadoAtual == ATENCAO || estadoAtual == CRITICO) && !frioExcessivo;
  digitalWrite(PINO_RELE, ligar ? HIGH : LOW);
}

// Buzzer com padrão por estado, sem usar delay()
void atualizarBuzzer()
{
  unsigned long t = millis();
  int freq = 0;
  switch (estadoAtual)
  {
  case ATENCAO:
    if (t % 2000 < 150)
      freq = 1000;
    break; // bip curto a cada 2 s
  case CRITICO:
    if (t % 500 < 300)
      freq = 2000;
    break; // alarme rápido
  case FALHA_SENSOR:
    if (t % 1000 < 100)
      freq = 500;
    break; // bip grave a cada 1 s
  default:
    break;
  }
  if (freq != freqBuzzerAtual)
  {
    if (freq == 0)
      noTone(PINO_BUZZER);
    else
      tone(PINO_BUZZER, freq);
    freqBuzzerAtual = freq;
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(PINO_RELE, OUTPUT);
  pinMode(PINO_BUZZER, OUTPUT);
  digitalWrite(PINO_RELE, LOW);
  analogReadResolution(12);
  dht.begin();

  Serial.println();
  Serial.println("=== FrioLog | No de borda da camara-piloto ===");
  Serial.println("Decisao 100% local no ESP32 (sem rede)");
  Serial.println("Faixa ideal: 2-8 C | umidade <= 70% | porta fechada (<100 lux)");
  Serial.println();
}

void loop()
{
  if (millis() - ultimaLeitura >= INTERVALO_LEITURA)
  {
    ultimaLeitura = millis();

    // --- Leitura dos sensores ---
    float temp = dht.readTemperature(); // DHT22: sensor digital (protocolo 1 fio)
    float umid = dht.readHumidity();
    int ldrBruto = analogRead(PINO_LDR); // LDR: sensor analógico (ADC 0-4095)
    float lux = lerLux();

    // --- Decisão local ---
    String novoMotivo;
    bool novoFrio;
    Estado novoEstado = decidirEstado(temp, umid, lux, novoMotivo, novoFrio);

    if (novoEstado != estadoAtual)
    {
      Serial.printf(">>> MUDANCA DE ESTADO: %s -> %s | motivo: %s\r\n",
                    nomeEstado(estadoAtual), nomeEstado(novoEstado), novoMotivo.c_str());
    }
    estadoAtual = novoEstado;
    motivoAtual = novoMotivo;
    frioExcessivo = novoFrio;

    // --- Atuação ---
    atualizarRele();

    Serial.printf("[%7lus] T=%5.1f C | U=%5.1f %% | Luz=%7.1f lux (ADC=%4d) | Estado=%-12s | Rele=%s | %s\r\n",
                  millis() / 1000, temp, umid, lux, ldrBruto,
                  nomeEstado(estadoAtual),
                  digitalRead(PINO_RELE) ? "LIGADO " : "DESLIG.",
                  motivoAtual.c_str());
  }

  atualizarBuzzer();
}
