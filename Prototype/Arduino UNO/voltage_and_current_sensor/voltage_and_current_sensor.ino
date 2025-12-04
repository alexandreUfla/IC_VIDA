// CONFIGURAÇÕES DO SISTEMA

// Tensão de referência do ADC
const float Vref = 5.0;

// Resolução do ADC do Arduino (10 bits → 0 a 1023)
const float ADCmax = 1023.0;


// GANHO DO AMPOP DO MEDIDOR DE TENSÃO
// Amplificador não-inversor:
// G = 1 + (Rf / Rin)
// Rf = 22k, Rin = 1k
const float Gv = 1.0 + (22000.0 / 1000.0);  // aproximadamente 23.0


// DIVISOR RESISTIVO DA BATERIA
// Divisor: 1k (entrada) + 160 ohms (para o GND)
// Fator do divisor:
const float Rtop = 1000.0;   // resistor da bateria até o nó do DR
const float Rbottom = 160.0; // resistor para o GND
const float DR_factor = (Rtop + Rbottom) / Rbottom;  // aproximadamente 7.25


// GANHO DO AMPOP DO MEDIDOR DE CORRENTE
// Amplificador diferencial:
// G = Rfeedback / Rin
// Rf = 33k, Rin = 1k
const float Gi = 33000.0 / 1000.0;  // aproximadamente 33.0


// CONSTANTE DO SHUNT (RESISTÊNCIA)
// 75 mV → 300 A   →   0.00025 V por ampere
const float shunt_mV = 75e-3;
const float shunt_A  = 300.0;
const float R_shunt = shunt_mV / shunt_A;  // 0.00025 V/A


void setup() {
  Serial.begin(9600);
}


void loop() {

  // LEITURA DA TENSÃO DA BATERIA
  int rawA0 = analogRead(A0);
  float VA0 = (rawA0 * Vref) / ADCmax;   // tensão no pino A0

  // Sinal antes do ampop (no DR)
  float V_DR = VA0 / Gv;

  // Tensão real da bateria
  float Vbat = V_DR * DR_factor;


  // LEITURA DA CORRENTE
  int rawA1 = analogRead(A1);
  float VA1 = (rawA1 * Vref) / ADCmax;  // tensão no pino A1

  // Tensão real no shunt
  float Vshunt = VA1 / Gi;

  // Corrente real (A)
  float Ishunt = Vshunt / R_shunt;


  // MOSTRAR RESULTADOS
  Serial.print("Tensão na bateria: ");
  Serial.print(Vbat);
  Serial.println(" V");

  Serial.print("Corrente no shunt: ");
  Serial.print(Ishunt);
  Serial.println(" A");

  Serial.println("------------------");
  delay(500);
}
