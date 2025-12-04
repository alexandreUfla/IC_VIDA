#include <WiFi.h>
#include <WebServer.h>

// WIFI
const char* ssid = "SEU_WIFI";
const char* password = "SUA_SENHA";

WebServer server(80);

// SISTEMA
const float Vref = 3.3;
const float ADCmax = 4095.0;

// Ganho tensão
const float Gv = 1.0 + (22000.0 / 1000.0);
const float Rtop = 1000.0;
const float Rbottom = 160.0;
const float DR_factor = (Rtop + Rbottom) / Rbottom;

// Ganho corrente
const float Gi = 33000.0 / 1000.0;
const float shunt_VperA = 75e-3 / 300.0;

// BATERIA
const float BAT_CAPACITY_Ah = 100.0;  // <==== COLOQUE SUA CAPACIDADE AQUI
float Ah_total = 0;
float Wh_total = 0;
unsigned long lastMillis = 0;

// JSON
String gerarJSON(
  float Vbat, float Ishunt, float Potencia,
  float Ah, float Wh,
  float SOC, float tempo_descarga,
  float tempo_15, String alerta
) {

  String json = "{";
  json += "\"tensao_bateria\": " + String(Vbat, 3) + ",";
  json += "\"corrente_shunt\": " + String(Ishunt, 3) + ",";
  json += "\"potencia\": " + String(Potencia, 3) + ",";
  json += "\"Ah\": " + String(Ah, 4) + ",";
  json += "\"Wh\": " + String(Wh, 4) + ",";
  json += "\"SOC_percent\": " + String(SOC, 2) + ",";
  json += "\"tempo_descarga_horas\": " + String(tempo_descarga, 2) + ",";
  json += "\"tempo_para_15porcento_horas\": " + String(tempo_15, 2) + ",";
  json += "\"alerta\": \"" + alerta + "\"";
  json += "}";

  return json;
}


void handleData() {

  // LEITURA TENSÃO
  int rawA0 = analogRead(34);
  float Vbat = ((rawA0 * Vref) / ADCmax) / Gv * DR_factor;

  // LEITURA CORRENTE
  int rawA1 = analogRead(35);
  float Ishunt = (((rawA1 * Vref) / ADCmax) / Gi) / shunt_VperA;

  // POTÊNCIA
  float Potencia = Vbat * Ishunt;

  // ENERGIA
  unsigned long agora = millis();
  float dt_horas = (agora - lastMillis) / 3600000.0;
  lastMillis = agora;

  Ah_total += Ishunt * dt_horas;
  Wh_total += Potencia * dt_horas;

  // CÁLCULO DO SOC (%)
  float Ah_consumidos = Ah_total;
  float Ah_restantes = BAT_CAPACITY_Ah - Ah_consumidos;

  if (Ah_restantes < 0) Ah_restantes = 0;

  float SOC = (Ah_restantes / BAT_CAPACITY_Ah) * 100.0;

  // TEMPO RESTANTE ATÉ DESCARGA
  float tempo_descarga = -1;

  if (Ishunt > 0.1)  // só calcula se estiver descarregando
    tempo_descarga = Ah_restantes / Ishunt;

  // TEMPO PARA ATINGIR 15%
  float Ah_limite_15 = BAT_CAPACITY_Ah * 0.15;
  float Ah_para_15 = Ah_restantes - Ah_limite_15;

  float tempo_15 = -1;

  if (Ishunt > 0.1 && Ah_para_15 > 0)
    tempo_15 = Ah_para_15 / Ishunt;

  // ALERTA
  String alerta = "";
  if (SOC <= 15.0)
    alerta = "BATERIA ABAIXO DE 15% - ATENCAO!";

  // JSON
  String json = gerarJSON(
    Vbat, Ishunt, Potencia,
    Ah_total, Wh_total,
    SOC, tempo_descarga,
    tempo_15, alerta
  );

  server.send(200, "application/json", json);
}


void setup() {
  Serial.begin(115200);
  analogReadResolution(12);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado!");
  Serial.print("IP ESP32: ");
  Serial.println(WiFi.localIP());

  lastMillis = millis();

  server.on("/data", handleData);
  server.begin();
}

void loop() {
  server.handleClient();
}
