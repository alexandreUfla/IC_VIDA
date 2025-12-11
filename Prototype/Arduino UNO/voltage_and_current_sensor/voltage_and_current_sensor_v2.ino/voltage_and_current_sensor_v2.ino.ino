// ------------------------------------------------------------
// LEITURA DE TENSÃO E CORRENTE PARA SEU CIRCUITO COM OPA350
// Shunt: 75mV @ 300A  → sensibilidade: 0.00025 V/A
// Ganho do amplificador: G = 33 (R13/R11 = 33k/1k)
// Divisor da bateria: 22k / 1k  → fator = 23
// Resistores de filtro trocados: R1 = 10k, R2 = 1k6 (sem impacto DC)
// ------------------------------------------------------------

const int pinBateria = A0;   // Entrada do medidor de tensão
const int pinShunt   = A1;   // Entrada do medidor de corrente

// --- Parâmetros do circuito ---
const float Rsuperior = 22000.0;   // 22k
const float Rinferior = 1000.0;    // 1k

const float gainShunt = 33.0;      // OPA350 (33k / 1k)
const float shuntSensitivity = 0.00025;  // 75mV / 300A

// --- Média para estabilizar ---
const int numSamples = 10;

void setup() {
  Serial.begin(9600);
}

float lerADC(int pino) {

  // leitura extra para carregar capacitor do ADC
  analogRead(pino);
  delayMicroseconds(5);

  long soma = 0;

  for (int i = 0; i < numSamples; i++) {
    soma += analogRead(pino);
    delayMicroseconds(50); // pequeno intervalo
  }

  return (float)soma / numSamples;
}

void loop() {

  // --- LEITURAS CORRIGIDAS ---
  float leituraBat   = lerADC(pinBateria);
  float leituraShunt = lerADC(pinShunt);

  // Converte de ADC para tensão (0–5 V)
  float VadcBat   = leituraBat   * (5.0 / 1023.0);
  float VadcShunt = leituraShunt * (5.0 / 1023.0);

  // Tensao real da bateria
  float Vbateria = VadcBat * ((Rsuperior + Rinferior) / Rinferior);

  // Tensão real no shunt (antes do amplificador)
  float VshuntReal = VadcShunt / gainShunt;

  // Corrente real
  float corrente = VshuntReal / shuntSensitivity;

  // Saída serial
  Serial.print("Tensao da Bateria: ");
  Serial.print(Vbateria, 2);
  Serial.println(" V");

  Serial.print("Corrente: ");
  Serial.print(corrente, 2);
  Serial.println(" A");

  Serial.println("-----------------------------");
  delay(200);
}
