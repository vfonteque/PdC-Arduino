#include <LiquidCrystal.h>
#include <string.h>
int contador = 0;
int pause1s = 1000;
int pause2s = 2000;
int pause3s = 3000;
int pause5s = 5000;
bool selected = false;
bool ar = true;

#define entradaAnalogica A2
int valorDeFuncionamento = 990;

#define pinBotoes A0

#define pinRs 8
#define pinEn 9
#define pinD4 4
#define pinD5 5
#define pinD6 6
#define pinD7 7
#define pinBackLight 10

#define pin1 13
#define pin2 12
#define pin3 11
#define pin4 3
#define pin5 2

#define btNENHUM 0
#define btSELECT 1
#define btLEFT   2
#define btUP     3
#define btDOWN   4
#define btRIGHT  5

#define tempoDebounce 50
#define erro 6

unsigned long delayBotao;
int estadoBotaoAnt = btNENHUM;

void estadoBotao(int botao);
void botaoApertado(int botao);

LiquidCrystal lcd(pinRs, pinEn, pinD4, pinD5, pinD6, pinD7);

String descBotao[6] = {"", "Select", "Esquerda", "Baixo", "Cima", "Direita"};

void setup() {
  Serial.begin(9600);
  pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT);
  pinMode(pin3, OUTPUT);
  pinMode(pin4, OUTPUT);
  pinMode(pin5, OUTPUT);
  pinMode(A1, INPUT);
  pinMode(pinBackLight, OUTPUT);
  digitalWrite(pinBackLight, HIGH);
  lcd.begin(16, 2);
}

void loop() {
  botaoSelecionado();

  switch (contador) {
    case 0:
      lcd.setCursor(0, 0);
      lcd.print("Inicio");
      if (selected) contador = 1;
      selected = false;
      break;
    case 1: // Pressao do ar
      if (ar) {
        lcd.clear();
        lcd.print("Pressao do ar OK");
        loading();
        Serial.println("Pressao de ar");
        leituraSerial();
        contador = 2;
        break;
      } else {
        contador = erro;
        lcd.clear();
        break;
      }
    case 2: // Purga
      if (ar) {
        lcd.clear();
        lcd.print("Purgando sistema");
        loading();
        Serial.println("Purga");
        leituraSerial();
        contador = 3;
        break;
      } else {
        contador = erro;
        lcd.clear();
        break;
      }
    case 3: // Ignicao 
      lcd.clear();
      Serial.println("Antes da ignicao");
      leituraSerial();
      lcd.print("Ignicao");
      Serial.println("Pos ignicao");
      leituraSerial();
      if (analogRead(entradaAnalogica) > valorDeFuncionamento) { // Primeira verificacao de chama antes da hora
        erroPreChama();
        break;
      }
      delay(pause3s);
      Serial.println("Pre valvula de gas");
      leituraSerial();
      if (analogRead(entradaAnalogica) > valorDeFuncionamento) { // Segunda verificacao
        erroPreChama();
        break;
      }
      contador = 4;
      break;
    case 4: // Valvula de gas
      lcd.clear();
      lcd.print("Abre valvula");
      delay(pause2s);
      Serial.println("Pos valvula");
      leituraSerial();
      if (analogRead(entradaAnalogica) < valorDeFuncionamento) { // Verificacao se a chama existe
        lcd.clear();
        lcd.print("SEM CHAMA");
        delay(pause2s);
        contador = erro;
        lcd.clear();
        break;
      }
      for (int i = 0; i < 3; i++) { // Teve chama inicial
        if (analogRead(entradaAnalogica) < valorDeFuncionamento) { // Verificacao se a chama se manteve 
          lcd.clear();
          lcd.print("APAGOU");
          Serial.println("APAGOU");
          delay(pause2s);
          contador = erro;
          lcd.clear();
          break;
        }
        delay(pause1s);
      }
      contador = 5;
      lcd.clear();
      break;
    case 5: // Funcionamento
      if (analogRead(entradaAnalogica) < valorDeFuncionamento) { // Verificacao se a chama esta normal
        contador = erro;
        lcd.clear();
        break;
      }
      lcd.setCursor(0, 0);
      lcd.print("Funcionando");
      if (selected) { // Desligamento do sistema
        lcd.clear();
        contador = 0;
        selected = false;
        lcd.print("Parando");
        delay(pause2s);
        lcd.clear();
      }
      break;
    case 6: // Trantamento de erros
      lcd.setCursor(0, 0);
      lcd.print("ERRO GERAL");
      delay(pause5s);
      lcd.clear();
      contador = 0;
      break;
    default:
      lcd.setCursor(0, 0);
      lcd.print("ERRO 204");
      break;
  }
}

void erroPreChama() {
  lcd.clear();
  lcd.print("CHAMA SEM ABRIR");
  lcd.setCursor(0, 1);
  lcd.print("VALVULA DE GAS");
  delay(pause2s);
  contador = erro;
  lcd.clear();
}

void loading() {
  lcd.setCursor(0, 1);
  lcd.print(".....");
  delay(pause2s);
  lcd.setCursor(0, 1);
  for (int i=0; i < 4; i++) {
    lcd.print("o");
    delay(pause2s);
  }
}

void leituraSerial() {
  Serial.println(digitalRead(entradaAnalogica));
  Serial.println(analogRead(entradaAnalogica));
}

void botaoSelecionado() {
   int valBotoes = analogRead(pinBotoes);
  if ((valBotoes < 800) && (valBotoes >= 600)) {
     estadoBotao(btSELECT);
     
  } else if ((valBotoes < 600) && (valBotoes >= 400)) {
     estadoBotao(btLEFT);
     
  } else if ((valBotoes < 400) && (valBotoes >= 200)) {
     estadoBotao(btUP);
     
  } else if ((valBotoes < 200) && (valBotoes >= 60)) {
     estadoBotao(btDOWN);
     
  } else if  (valBotoes < 60) {
     estadoBotao(btRIGHT);
     
  } else {
     estadoBotao(btNENHUM);
  }
}

void estadoBotao(int botao) {
  if ((millis() - delayBotao) > tempoDebounce) {
     if ((botao != btNENHUM) && (estadoBotaoAnt == btNENHUM) ) {
        botaoApertado(botao); 
        delayBotao = millis();
     }
     if ((botao == btNENHUM) && (estadoBotaoAnt != btNENHUM) ) {
        delayBotao = millis();
     }
  }
  estadoBotaoAnt = botao;
}

void botaoApertado(int botao) {
  if(botao == 5) {} // DIREITA
  if(botao == 2) {} // ESQUERDA
  if(botao == 3) {} // CIMA
  if(botao == 4) {} // BAIXO
  if(botao == 1) { // SELECT
    selected = true;
  }
}
