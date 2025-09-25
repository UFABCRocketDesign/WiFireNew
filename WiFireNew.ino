// Definições de modos de operação
#define PROPER_BOARD 1                          // 1 Para quando estiver usando a placa Wifire com botoes
  #define NEW_BOARD (PROPER_BOARD && 1)           // 1 Para quando estiver usando a placa TE 2025 (com botoes)
    #define CONTINUIDADE (NEW_BOARD && !RELE && 1)  // Liga/Desliga verificação de continuidade do ignitor


#define LORA 1     // Liga o LoRa

#define BUZZER 1   // Liga o buzzer
  #define MILLIS (1 && BUZZER) // Alterna entre apitos com delay ou com millis

#define LED_RGB (PROPER_BOARD && 1)  // Liga os leds RGB

//-----------------------------Modos de Ignição------------------------------------------------------------
#define RELE (PROPER_BOARD && 0)                // Alterna entre modulo rele(porta 27) ou mosfet(porta A8 com NEW_BOARD = 1 e porta A0 caso seja NEW_BOARD = 0)
#define IGN1234 (!PROPER_BOARD && 0)            // Alterna entre as 4 portas da placa de voo (=1) ou a porta IGNN 27 (=0),




//-----------------------------------------------------------------------------------------
#if BUZZER
#if (MILLIS)
#define BEEP_D 0  // Apitos com delay
#define BEEP_M 1  // Apitos com millis
#else
#define BEEP_D 1  // Apitos com delay
#define BEEP_M 0  // Apitos com millis
#endif            // MILLIS
#endif            // BUZZER

#if (PROPER_BOARD)
// Pinos Ign
#if (RELE)
#define IGNN 27
#else
#if NEW_BOARD
#define IGNN A8
#else
#define IGNN A0
#endif
#endif  // RELE

#else
// Pinos Ign
#if (IGN1234)
//Usar quando estiver com a placa principal ou a antiga para o acionamento dos skibs
#define IGN1 A0 /*act1*/
#define IGN2 A2 /*act2*/
#define IGN3 A4 /*act3*/
#define IGN4 A6 /*act4*/
#else
#define IGNN 27
#endif  // IGN1234
#endif  // PROPER_BOARD

#define ignOn 1
//-----------------------------------------------------------------------------------------
#if (PROPER_BOARD)
// Definição dos pinos dos botões
#define BUTTON_TYPE INPUT_PULLUP  // Ou INPUT

#if NEW_BOARD
#if (BUTTON_TYPE == INPUT_PULLUP)
#define B_ON LOW
#else
#define B_ON HIGH
#endif
#else
#if (BUTTON_TYPE == INPUT_PULLUP)
#define B_ON HIGH
#else
#define B_ON LOW
#endif
#endif


#if NEW_BOARD
#define B 3  // BLUE
#define G 4  // GREEN
#define Y 5  // YELLOW
#define R 6  // RED
#if CONTINUIDADE
#define CONT_PIN A9
#define CONT_THRESHOLD 200
#endif

#else
#define B 24  // BLUE
#define G 28  // GREEN
#define Y 30  // YELLOW
#define R 26  // RED
#endif

#endif  // PROPER_BOARD
//-----------------------------------------------------------------------------------------
#if (LED_RGB)
#define L_ON LOW

#define LEDR 32
#define LEDG 36
#define LEDB 34

enum Colors { white = 0b111,
              magenta = 0b101,
              blue = 0b001,
              cyan = 0b011,
              green = 0b010,
              yellow = 0b110,
              red = 0b100,
              black = 0b000 };
#endif  // LED_RGB
//-----------------------------------------------------------------------------------------
#if (LORA)
// Configuração do LoRa via Serial3 ou SoftwareSerial
#if defined(ARDUINO_AVR_MEGA2560)  // Se for um Arduino MEGA
#if NEW_BOARD
HardwareSerial &LoRa = Serial2;
// Pinos adicionais do LoRa
#define M0_LORA_PIN 12
#define M1_LORA_PIN 11
#define AUX_LORA_PIN 10
#else
HardwareSerial &LoRa = Serial3;
// Pinos adicionais do LoRa
#define M0_LORA_PIN 12
#define M1_LORA_PIN 11
#define AUX_LORA_PIN A8
#endif

#else
#include <SoftwareSerial.h>
SoftwareSerial LoRa(2, 3);  // Se for um Arduino sem Serial3, usa SoftwareSerial
                            // Pinos adicionais do LoRa
#define M0_LORA_PIN 12
#define M1_LORA_PIN 11
#define AUX_LORA_PIN A8
#endif


#endif  // LORA
//-----------------------------------------------------------------------------------------
#if (BEEP_D || BEEP_M)
#define buzzPin 22

#if NEW_BOARD
#define buzzOn HIGH
#else
#define buzzOn LOW
#endif

#define intervalBeepT 150
#define durationBeepT 20
#define intervalBeepR 50
#define durationBeepR 50
#endif  // BUZZER
//-----------------------------------------------------------------------------------------
#if (PROPER_BOARD)
// Configurações de Senha
#define password_len 5
String password[password_len] = { "b", "g", "b", "b", "g" };
String key[password_len];
int keyIndex = 0;
#endif  // PROPER_BOARD
//-----------------------------------------------------------------------------------------
// Variáveis de controle do sistema
bool systemArmed = false;
bool armedSent = false;            // Variável para evitar envio repetido
bool printedArmedMessage = false;  // Evita imprimir mensagem repetidamente

unsigned long time = 0;
//-----------------------------------------------------------------------------------------
#if (BEEP_M)
class Timer {
private:
  int pin;
  bool pinOn;
  unsigned long interval;
  unsigned long onDuration;
  bool beepOnTone;

  bool pinSeal = false;
  int maxTimes = 0;
  int timesCount = 0;
  unsigned long previousMillis = 0;

public:
  Timer(int pino, bool on, unsigned long intervalo, unsigned long duracao, bool toneMode = false)
    : pin(pino), pinOn(on), interval(intervalo), onDuration(duracao), beepOnTone(toneMode) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, !pinOn);
  }

  void loop(unsigned long time) {
    seal();
    if (pinSeal) {
      if (timesCount < maxTimes) {
        if (time <= previousMillis + onDuration) {
          if (!beepOnTone) {
            digitalWrite(pin, pinOn);
          } else {
            tone(pin, 1000);
          }
        } else if (time > previousMillis + onDuration && time <= previousMillis + onDuration + interval) {
          if (!beepOnTone) {
            digitalWrite(pin, !pinOn);
          } else {
            noTone(pin);
          }
        } else {
          previousMillis = time;
          timesCount += 1;
        }
      } else {
        timesCount = 0;
        maxTimes = 0;
        unseal();
      }
    }
  }

  void add() {
    if (pinSeal == false) {
      maxTimes += 1;
    }
  }

private:

  void seal() {
    if (!pinSeal && maxTimes != 0) {
      previousMillis = millis();
      pinSeal = true;
    }
  }

  void unseal() {
    pinSeal = false;
  }
};

Timer beepT(buzzPin, buzzOn, intervalBeepT, durationBeepT);
Timer beepR(buzzPin, buzzOn, intervalBeepR, durationBeepR);
#endif  // BEEP_M
//-----------------------------------------------------------------------------------------
#if (LED_RGB)
// Função para acionar os LEDs RGB com valores booleanos
void rgb(bool r, bool g, bool b) {
  digitalWrite(LEDR, r);
  digitalWrite(LEDG, g);
  digitalWrite(LEDB, b);
}

// Função para definir a cor pelo enum Colors
void setColor(Colors color) {
  switch (color) {
    case white: rgb(L_ON, L_ON, L_ON); break;
    case magenta: rgb(L_ON, !L_ON, L_ON); break;
    case blue: rgb(!L_ON, !L_ON, L_ON); break;
    case cyan: rgb(!L_ON, L_ON, L_ON); break;
    case green: rgb(!L_ON, L_ON, !L_ON); break;
    case yellow: rgb(L_ON, L_ON, !L_ON); break;
    case red: rgb(L_ON, !L_ON, !L_ON); break;
    case black: rgb(!L_ON, !L_ON, !L_ON); break;
  }
}
#endif  // LED_RGB
//-----------------------------------------------------------------------------------------
#if (BEEP_D)
void buzzerONOF(int tempo, int apitos) {
  for (int i = 0; i < apitos; i++) {
    digitalWrite(buzzPin, buzzOn);
    delay(tempo);
    digitalWrite(buzzPin, !buzzOn);
    delay(tempo);
  }
}
#endif  // BEEP_D
//-----------------------------------------------------------------------------------------
#if (PROPER_BOARD)
void resetSenha() {
  Serial.println("Senha resetada");
  keyIndex = 0;
  for (int i = 0; i < password_len; i++) {
    key[i] = "";
  }
  armedSent = false;
  printedArmedMessage = false;
}
#endif  // PROPER_BOARD
//-----------------------------------------------------------------------------------------
#if (PROPER_BOARD)
bool verificarSenha() {
  for (int i = 0; i < password_len; i++) {
    if (key[i] != password[i]) {
      return false;
    }
  }
  return true;
}
#endif  // PROPER_BOARD
//-----------------------------------------------------------------------------------------
void processarMensagem(String mensagem) {
  mensagem.trim();
  if (mensagem == "PING!") {
    Serial.println("Ping '*'");
    systemArmed = false;
#if (LORA)
#if CONTINUIDADE
    Serial.println(analogRead(CONT_PIN));
    if (analogRead(CONT_PIN) >= CONT_THRESHOLD) {
      transmit("PONG1\n");
    } else {
      transmit("PONG0\n");
    }
#else
    transmit("PONG0\n");
#endif
#endif  // LORA
#if (BEEP_D)
    buzzerONOF(100, 3);
#endif  // BEEP_D
#if (BEEP_M)
    beepR.add();
    beepR.add();
    beepR.add();
#endif  // BEEP_M
  }
  if (mensagem == "S_PING!") {
    Serial.println("Enviando Ping '*'");
    systemArmed = false;
#if (LORA)
    transmit("PING!\n");
#endif  // LORA
  }
  if (mensagem == "DISARMED!") {
    Serial.println("Desarmado");
    systemArmed = false;
// #if LORA
//     transmit("DISARMED!\n");
// #endif
  }
  if (mensagem == "PONG0") {
    Serial.println("PONG0");
    systemArmed = false;
#if (BEEP_D)
    buzzerONOF(100, 3);
#endif  // BEEP_D
#if (BEEP_M)
    beepR.add();
    beepR.add();
    beepR.add();
#endif  // BEEP_M
  }
  if (mensagem == "PONG1") {
    Serial.println("PONG1");
    systemArmed = false;
#if (BEEP_D)
    buzzerONOF(100, 3);
#endif  // BEEP_D
#if (BEEP_M)
    beepR.add();
    beepR.add();
    beepR.add();
#endif  // BEEP_M
  }
  if (mensagem == "ARMED!") {
    systemArmed = true;
    armedSent = true;
    Serial.println("Sistema Armado!");
// #if LORA
//     transmit("ARMED!\n");
// #endif
  }
    if (mensagem == "S_ARMED!") {
      Serial.println("Enviando ARMED");
  #if LORA
      transmit("ARMED!\n");
  #endif
}
  if (mensagem == "S_IGN!") {
    Serial.println("Enviando IGN");
#if LORA
    transmit("IGN!\n");
#endif
  }
if (mensagem == "IGN!" && systemArmed) {
  Serial.println("Ignição!");
#if LORA
  transmit("IGN!\n");
#endif
#if (LED_RGB)
  setColor(red);
#endif  // LED_RGB
  systemArmed = false;

#if (PROPER_BOARD)
  digitalWrite(IGNN, ignOn);
  delay(3000);
  digitalWrite(IGNN, !ignOn);
  resetSenha();

#else
#if IGN1234
  //Usar com placa de acionamento antiga
  digitalWrite(IGN1, ignOn);
  digitalWrite(IGN2, ignOn);
  digitalWrite(IGN3, ignOn);
  digitalWrite(IGN4, ignOn);
  delay(3000);
  digitalWrite(IGN1, !ignOn);
  digitalWrite(IGN2, !ignOn);
  digitalWrite(IGN3, !ignOn);
  digitalWrite(IGN4, !ignOn);
#else
  digitalWrite(IGNN, ignOn);
  delay(3000);
  digitalWrite(IGNN, !ignOn);
#endif  // IGN1234
#endif  // PROPER_BOARD

}
}
//-----------------------------------------------------------------------------------------
#if (LORA)
// Função para ler mensagens do LoRa via Serial
void lerLoRa() {
  if (LoRa.available()) {
    String mensagemLoRa = LoRa.readStringUntil('\n'); // lê até '\n'
    Serial.print("Recebido via LoRa: ");
    Serial.println(mensagemLoRa);
    processarMensagem(mensagemLoRa);
  }
}



// Função para transmitir mensagens via LoRa
void transmit(String msg) {
  LoRa.print(msg);
}
#endif  // LORA
//-----------------------------------------------------------------------------------------
// Função para ler dados da Serial
void lerSerial() {
  if (Serial.available()) {
    String mensagemSerial = Serial.readString();
    Serial.print("Recebido via Serial: ");
    Serial.println(mensagemSerial);
    processarMensagem(mensagemSerial);
  }
}

//-----------------------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);

#if (PROPER_BOARD)
  pinMode(B, BUTTON_TYPE);
  pinMode(G, BUTTON_TYPE);
  pinMode(Y, BUTTON_TYPE);
  pinMode(R, BUTTON_TYPE);

  pinMode(IGNN, OUTPUT);
#else
#if IGN1234
  //Usar com placa de acionamento antiga
  pinMode(IGN1, OUTPUT);
  pinMode(IGN2, OUTPUT);
  pinMode(IGN3, OUTPUT);
  pinMode(IGN4, OUTPUT);
#else
  pinMode(IGNN, OUTPUT);
#endif  // IGN1234
#endif  // PROPER_BOARD

#if (RELE)
  pinMode(A0, OUTPUT);
  digitalWrite(A0, HIGH);
#endif  // RELE

#if (LORA)
  LoRa.begin(9600);
  pinMode(M0_LORA_PIN, OUTPUT);
  pinMode(M1_LORA_PIN, OUTPUT);
  pinMode(AUX_LORA_PIN, INPUT);

  digitalWrite(M0_LORA_PIN, LOW);
  digitalWrite(M1_LORA_PIN, LOW);
#endif  // LORA

#if (BEEP_D)
  pinMode(buzzPin, OUTPUT);
  buzzerONOF(200, 1);
#endif  // BEEP_D

#if (BEEP_M)
  pinMode(buzzPin, OUTPUT);
  digitalWrite(buzzPin, buzzOn);
  delay(300);
  digitalWrite(buzzPin, !buzzOn);
#endif  // BEEP_M

#if (LED_RGB)
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);

  setColor(white);
  delay(100);
  setColor(magenta);
  delay(100);
  setColor(blue);
  delay(100);
  setColor(cyan);
  delay(100);
  setColor(green);
  delay(100);
  setColor(yellow);
  delay(100);
  setColor(red);
  delay(100);
  setColor(black);
  delay(100);
#endif  // LED_RGB

  Serial.println("Sistema inicializado!");
}
//-----------------------------------------------------------------------------------------
void loop() {
  time = millis();
#if (LORA)
  lerLoRa();
#endif  // LORA
  lerSerial();
//-----------------------------------------------------------------------------------------
#if (LED_RGB)
  setColor(black);
#endif  // LED_RGB
//-----------------------------------------------------------------------------------------
#if (PROPER_BOARD)
  if (keyIndex >= password_len) {
    keyIndex = 0;
    if (!verificarSenha()) {
      resetSenha();
      setColor(red);
      delay(100);
      setColor(black);
      delay(100);
      setColor(red);
      delay(100);
      setColor(black);
      delay(100);
      setColor(red);
      delay(100);
      setColor(black);
    }
  }

  if (verificarSenha()) {
    if (!systemArmed) {
      systemArmed = true;
      armedSent = false;
    }
  }
#endif  // PROPER_BOARD

  if (systemArmed) {
#if (LED_RGB)
    setColor(magenta);
#endif  // LED_RGB
#if (BEEP_D)
    buzzerONOF(75, 3);
#endif  // BEEP_D
#if (BEEP_M)
    beepT.add();
    beepT.add();
    beepT.add();
#endif  // BEEP_M
  }

#if (PROPER_BOARD)
  // Envia a mensagem "A" apenas uma vez quando arma o sistema
  if (systemArmed && !armedSent) {
#if (LORA)
    transmit("ARMED!\n");
#endif  // LORA
    armedSent = true;
  }

  // Exibe a mensagem de sistema armado apenas uma vez
  if (systemArmed && !printedArmedMessage) {
    Serial.println("Sistema Armado! (aguardando comando)");
    printedArmedMessage = true;
  }
#endif  // PROPER_BOARD
//-----------------------------------------------------------------------------------------
#if (PROPER_BOARD)
  // Botão Azul (B)
  if (digitalRead(B) == B_ON) {
    delay(50);
    if (digitalRead(B) == B_ON) {
      Serial.println("Botão Azul pressionado");
#if (BUZZER)
      digitalWrite(buzzPin, buzzOn);
#endif  // BUZZER
#if (LED_RGB)
      setColor(blue);
#endif  // LED_RGB
      while (digitalRead(B) == B_ON) {};
#if (BUZZER)
      digitalWrite(buzzPin, !buzzOn);
#endif  // BUZZER
      key[keyIndex] = "b";
      keyIndex++;
      for (int i = 0; i < password_len; i++) {
        Serial.print(key[i]);
        Serial.print(" ");
      }
      Serial.println("\n----------------------------");
    }
  }
#endif  // PROPER_BOARD
//-----------------------------------------------------------------------------------------
#if (PROPER_BOARD)
  // Botão Verde (G)
  if (digitalRead(G) == B_ON) {
    delay(50);
    if (digitalRead(G) == B_ON) {
      Serial.println("Botão Verde pressionado");
#if (BUZZER)
      digitalWrite(buzzPin, buzzOn);
#endif  // BUZZER
#if (LED_RGB)
      setColor(green);
#endif  // LED_RGB
      while (digitalRead(G) == B_ON) {};

#if (BUZZER)
      digitalWrite(buzzPin, !buzzOn);
#endif  // BUZZER
      key[keyIndex] = "g";
      keyIndex++;
      for (int i = 0; i < password_len; i++) {
        Serial.print(key[i]);
        Serial.print(" ");
      }
      Serial.println("\n----------------------------");
    }
  }
#endif  // PROPER_BOARD
//-----------------------------------------------------------------------------------------
#if (PROPER_BOARD)
  // Botão Amarelo (Y)
  if (digitalRead(Y) == B_ON) {
    delay(50);
    if (digitalRead(Y) == B_ON) {
      Serial.println("Botão Y pressionado! (ping/reset)");
#if (BUZZER)
      digitalWrite(buzzPin, buzzOn);
#endif  // BUZZER
#if (LED_RGB)
      setColor(yellow);
#endif  // LED_RGB
      while (digitalRead(Y) == B_ON) {};

#if (BUZZER)
      digitalWrite(buzzPin, !buzzOn);
#endif  // BUZZER
      resetSenha();
      systemArmed = false;
#if (LORA)
      transmit("PING!\n");
#endif  // LORA
      for (int i = 0; i < password_len; i++) {
        Serial.print(key[i]);
        Serial.print(" ");
      }
      Serial.println("\n----------------------------");
    }
  }
#endif  // PROPER_BOARD
//-----------------------------------------------------------------------------------------
#if (PROPER_BOARD)
  // Botão Vermelho (R)
  if (digitalRead(R) == B_ON) {
    delay(50);
    if (digitalRead(R) == B_ON) {
      Serial.println("Botão R pressionado! (ignição)");
#if (BUZZER)
      digitalWrite(buzzPin, buzzOn);
#endif  // BUZZER
#if (LED_RGB)
      setColor(red);
#endif  // LED_RGB
      while (digitalRead(R) == B_ON) {};
#if (BUZZER)
      digitalWrite(buzzPin, !buzzOn);
#endif  // BUZZER
      if (systemArmed) {
        Serial.println("Enviando ignição!");
#if (LORA)
        transmit("IGN!\n");
#endif  // LORA
        systemArmed = false;
#if (BUZZER)
        digitalWrite(buzzPin, buzzOn);
        delay(3000);
#endif  // BUZZER
        resetSenha();
#if (BUZZER)
        digitalWrite(buzzPin, !buzzOn);
        delay(200);
        digitalWrite(buzzPin, !buzzOn);
#endif  // BUZZER
      } else {
        resetSenha();
      }
      for (int i = 0; i < password_len; i++) {
        Serial.print(key[i]);
        Serial.print(" ");
      }
      Serial.println("\n----------------------------");
    }
  }
#endif
#if (BEEP_M)
  beepT.loop(time);
  beepR.loop(time);
#endif  // BEEP_M
}