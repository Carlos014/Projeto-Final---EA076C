//Sensor de luz

#include "TimerOne.h"

typedef struct {
  int tam_buffer = 0;
  char letter[14];
} buffer;

int portaLDR = A0; // Pino analógico utilizado pelo LDR.
int SensorValue = 0; //Valor lido do LDR
int timeON = 0, timeOFF = 0, flag = 0, edge = 0, n = 0, s = 0, Dit = 0, Dah = 0, initialize = 1, initword = 0;
buffer Morse;

void buffer_add(char c_in) {
  int i;
  Morse.letter[Morse.tam_buffer] = c_in;
  Morse.tam_buffer++;
}

void setup() {
  Timer1.initialize(5000);
  Timer1.attachInterrupt(counter);
  Serial.begin(9600); //Inicia a comunicação serial
}

int inrange(float a, float b, float pminor, float pmajor) {
  // Verifica se o tempo aceso e apagado medidos estão em um determinado intervalo para classificar como Dit ou Dah.
  // As variáveis pminor e pmajor servem para eliminar possiveis lacunas de tempo existentes entre Dit e Dah.
  if (a > ((100-pminor) * b / 100) && a < ((pmajor + 100) * b / 100) ) return 1;
  return 0;
}

void initCom() {
  if (edge) { // Detecta se houve uma variação na luz recebida pelo LDR.
    edge = 0; 

    if (flag) {
      if (timeOFF > 3000 || timeON == 0) {
        initword = 1; // Sai do estado de repouso da placa e se prepara para receber o caractere de sincronismo.
      }
      else {
        if (inrange(timeOFF, Dah, 30, 30)) {
          // Caso o caractere de sincronismo tenha sido enviado corretamente, este é utilizado para se determinar o Dah padrão.
          Dah = (Dah + timeOFF) / 2;
          Dit = Dah / 3;
          initialize = 0;
        }
        else {
          // Caso o caractere de sincronismo não tenha sido enviado corretamente, uma mensagem de erro é exibida.
          Serial.println("Nao foi possivel estabelecer o tempo base");
        }
      }
    }
    else {
      if (initword) {
        // Envia-se um caractere T de sincronismo para se configurar a comunicação.
        Dah = timeON; //tempo 3T.
        initword = 0;
      }
    }
  }
}

void getword() {
  // Estado padrão de leitura de caracteres.
  if (edge) {
    edge = 0;

    // Ao receber-se um sinal do LDR, estes são classificados como um Dit ou um Dah e uma ação apropriada correspondente é tomada.
    if (flag) {
      if (inrange(timeOFF, Dit, 35, 80)) ;// Do nothing.
      else if (inrange(timeOFF, Dah, 40, 60)) {
        // Ao final da transmissão de uma letra.
        buffer_add('\0');
        Printletter();
      }
      else if (inrange(timeOFF, 7 * Dit, 32, 50)) {
        // Ao final da transmissão de uma palavra.
        buffer_add('\0');
        Printletter();
        Serial.print(' ');
      }
      else if(timeOFF> 7*Dit) {
        // Caso ele permaneça muito tempo em repouso a comunicação e o buffer são reinicializados.
           Serial.print("\r\n");
           Morse.tam_buffer = 0;
      }
    }
    else {
      if (inrange(timeON, Dit, 35, 80)) buffer_add('.');
      else if (inrange(timeON, Dah, 40, 60)) buffer_add('-');
    }
  }
  if(Morse.tam_buffer > 4){
    Morse.tam_buffer = 0; // Caso haja algum problema na recepção da letra reinicializa-se o buffer.
    Serial.print('?');
  }
}

void counter() {
  // Para cada interrupção periódica de 5ms armazena-se o tempo que se passou e recebe-se o próximo sinal a ser convertido em uma letra. 
  if (initialize) initCom();
  else  getword();
  s++;
}

void loop() {
  ///ler o valor do LDR
  SensorValue = analogRead(portaLDR);

  // Em função da flag são executados apenas quando há transição de estados na recepção de luz.
  if (SensorValue > 800 && flag == 0 && s > 10) {
    // Caso o sensor tenha recebido luz.
    timeOFF = s;
    s = 0;
    flag = 1;
    edge = 1;
  } else if (SensorValue <= 800 && flag == 1 && s > 10) {
    // Caso o sensor deixe de receber luz depois de tê-la recebido.
    timeON = s;
    flag = 0;
    edge = 1;
    s = 0;
  }
}


void Printletter() {
  // Compara a sequência recebida de sinais (Dit e Dah) e os converte em uma letra.
  if (Morse.letter[0] == '.') {
    if (Morse.letter[1] == '.') {
      if (Morse.letter[2] == '.') {
        if (Morse.letter[3] == '.') {

          if (Morse.letter[4] == '\0') {
            Serial.print('H');
          }

        }
        if (Morse.letter[3] == '-') {

          if (Morse.letter[4] == '\0') {
            Serial.print('V');
          }
        }
        if (Morse.letter[3] == '\0') {
          Serial.print('S');
        }

      }
      if (Morse.letter[2] == '-') {
        if (Morse.letter[3] == '.') {

          if (Morse.letter[4] == '\0') {
            Serial.print('F');
          }

        }

        if (Morse.letter[3] == '\0') {
          Serial.print('U');
        }

      }
      if (Morse.letter[2] == '\0') {
        Serial.print('I');
      }
    }
    if (Morse.letter[1] == '-') {
      if (Morse.letter[2] == '.') {
        if (Morse.letter[3] == '.') {
          if (Morse.letter[4] == '\0') {
            Serial.print('L');
          }

        }
        if (Morse.letter[3] == '\0') {
          Serial.print('R');
        }
      }
      if (Morse.letter[2] == '-') {
        if (Morse.letter[3] == '.') {

          if (Morse.letter[4] == '\0') {
            Serial.print('P');
          }

        }
        if (Morse.letter[3] == '-') {

          if (Morse.letter[4] == '\0') {
            Serial.print('J');
          }

        }
        if (Morse.letter[3] == '\0') {
          Serial.print('W');
        }


      }
      if (Morse.letter[2] == '\0') {
        Serial.print('A');
      }
    }
    if (Morse.letter[1] == '\0') {
      Serial.print('E');
    }
  }


  if (Morse.letter[0] == '-') {
    if (Morse.letter[1] == '.') {
      if (Morse.letter[2] == '.') {
        if (Morse.letter[3] == '.') {

          if (Morse.letter[4] == '\0') {
            Serial.print('B');
          }
        }
        if (Morse.letter[3] == '-') {

          if (Morse.letter[4] == '\0') {
            Serial.print('X');
          }
        }
        if (Morse.letter[3] == '\0') {
          Serial.print('D');
        }
      }
      if (Morse.letter[2] == '-') {
        if (Morse.letter[3] == '.') {

          if (Morse.letter[4] == '\0') {
            Serial.print('C');
          }
       // if (voce realmente ta conferindo) {
       //   Serial.print('Olá!')
      
        }
        if (Morse.letter[3] == '-') {

          if (Morse.letter[4] == '\0') {
            Serial.print('Y');
          }

        }
        if (Morse.letter[3] == '\0') {
          Serial.print('K');
        }
      }
      if (Morse.letter[2] == '\0') {
        Serial.print('N');
      }
    }
    if (Morse.letter[1] == '-') {
      if (Morse.letter[2] == '.') {
        if (Morse.letter[3] == '.') {
          if (Morse.letter[4] == '\0') {
            Serial.print('Z');
          }
        }

        if (Morse.letter[3] == '-') {
          if (Morse.letter[4] == '\0') {
            Serial.print('Q');
          }

        }
        if (Morse.letter[3] == '\0') {
          Serial.print('G');
        }
      }
      if (Morse.letter[2] == '-') {
        if (Morse.letter[3] == '\0') {
          Serial.print('O');
        }
      }
      if (Morse.letter[2] == '\0') {
        Serial.print('M');
      }
    }
    if (Morse.letter[1] == '\0') {
      Serial.print('T');
    }

  }
  Morse.tam_buffer = 0; // Após a impressão da letra, o buffer é limpo e preparado para receber uma nova letra.
}



