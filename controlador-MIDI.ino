#include <LiquidCrystal.h>
#include <MIDI.h>


//Multiplexador
#define S0  8
#define S1  9
#define S2  10
#define S3  11
#define SIG 14

//Botões Axuliares
#define BT0 12
#define BT1 15
#define BT2 13
#define BT3 16

LiquidCrystal lcd(7, 6, 5, 4, 3, 2);
MIDI_CREATE_DEFAULT_INSTANCE();

////////////////////////////////////////////
//Caracter Especial

byte off[8] = {
  0b11111,
  0b10001,
  0b10001,
  0b10001,
  0b10001,
  0b10001,
  0b10001,
  0b11111
};

byte on[8] = {
  0b11111,
  0b11111,
  0b10101,
  0b11011,
  0b11011,
  0b10101,
  0b11111,
  0b11111
};

byte modo[8] = {
  0b11100,
  0b01110,
  0b01110,
  0b00111,
  0b00111,
  0b01110,
  0b01110,
  0b11100
};



/////////////////////////////////////////////

const int NButtons = 16; // *coloque aqui o numero de entradas digitais utilizadas
int buttonCState[NButtons] = {0}; // estado atual da porta digital
int buttonPState[NButtons] = {0}; // estado previo da porta digital

/////////////////////////////////////////////

const int NPots = 5; // *coloque aqui o numero de entradas analogicas utilizadas  
const int pot[NPots] = {A3,A4,A5,A6,A7}; // *neste array coloque na ordem desejada os pinos das portas analogicas utilizadas
int potCState[NPots] = {0}; // estado atual da porta analogica
int potPState[NPots] = {0}; // estado previo da porta analogica
int potVar = 5; // variacao entre o valor do estado previo e o atual da porta analogica

/////////////////////////////////////////////

byte midiCh = 1; // *Canal midi a ser utilizado
byte note = 4; // *Nota mais grave que sera utilizada - use essa linha se quiser usar uma escala cromatica
//byte note[NButtons] = {36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47}; // use essa linha se quiser usar uma escala personalizada
byte cc = 1; // *CC mais baixo que sera utilizado

/////////////////////////////////////////////

int TIMEOUT = 300; //quantidade de tempo em que o potenciometro sera lido apos ultrapassar o varThreshold
int varThreshold = 4; //threshold para a variacao no sinal do potenciometro
boolean potMoving = true; // se o potenciometro esta se movendo
unsigned long pTime[NPots] = {0}; // tempo armazenado anteriormente
unsigned long timer[NPots] = {0}; // armazena o tempo que passou desde que o timer foi zerado

/////////////////////////////////////////////

byte deslocamento = 2;


void setup () {
  //Inicialização MIDI
  MIDI.begin();
  Serial.begin(115200);
  
  //Inicializando os pinos
  //Multiplexador
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);  
  pinMode(SIG,  INPUT);

  //Botões Auxiliares
  pinMode(BT0,  INPUT);
  pinMode(BT1,  INPUT);
  pinMode(BT2,  INPUT);
  pinMode(BT3,  INPUT);

  
  //Analógicos
  for (int i=0; i<NPots; i++){
    pinMode(pot[i], INPUT);
  }

  lcd.createChar(0, off);
  lcd.createChar(1, on);
  lcd.createChar(2, modo);
  

  //Configuração LCD e Inicialização
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("DJ Controller");
  delay(1000);
  lcd.clear();
  display();
}

void loop () {
  if(digitalRead(BT0)){
    noteOFF();
    lcd.clear();
    while(digitalRead(BT0)){
      int x = btReturn();
      if(x<8){
        deslocamento = x;
      }
      displayModo();
      displayNotas();
    }    
    lcd.clear();    
    display();
  }
  
  if(digitalRead(BT1)){
    noteOFF();
    lcd.clear();
    while(digitalRead(BT1)){
      int x = btReturn();
      if(x<16){
        midiCh = x+1;
      }
      displayModo();
      displayCanal();
    }  
    lcd.clear();    
    display();
  }
  
  disparoBotoes();
  botao();
  disparoPotenciometros();
}

/////////////////////////////////////////////
//Telas

void display(){  
  displayCanal();
  displayNotas();
}

void displayCanal(){
  lcd.setCursor(0, 1);
  lcd.print("Canal:");
  if(midiCh<10){    
    lcd.print(" ");
  }
  lcd.print(midiCh);
}

void displayNotas(){   
  for(int i=0; i<4;i++){
    if((i+4) ==  deslocamento){      
      lcd.setCursor(i+12, 0);        
      lcd.write(byte(1)); 
    }else{         
      lcd.setCursor(i+12, 0);  
      lcd.write(byte(0));
    }
  }   
  for(int j=0; j<4;j++){
    if(j ==  deslocamento){      
      lcd.setCursor(j+12, 1);        
      lcd.write(byte(1)); 
    }else{         
      lcd.setCursor(j+12, 1);  
      lcd.write(byte(0));
    }
  }
}

void displayModo(){  
  lcd.setCursor(0, 0);  
  lcd.write(byte(2));
  lcd.write(byte(2));
  lcd.print("Modo"); 
  lcd.setCursor(0, 1); 
}

/////////////////////////////////////////////
//Operações

void multi(int v0,int v1, int v2, int v3){
  digitalWrite(S0, v0);
  digitalWrite(S1, v1);
  digitalWrite(S2, v2);
  digitalWrite(S3, v3);
}

void botao(){  
  multi(0,0,0,0);
  buttonCState[0] = digitalRead(SIG);
  multi(1,0,0,0);
  buttonCState[1] = digitalRead(SIG);
  multi(0,1,0,0);
  buttonCState[2] = digitalRead(SIG);
  multi(1,1,0,0);
  buttonCState[3] = digitalRead(SIG);
  multi(0,0,1,0);
  buttonCState[4] = digitalRead(SIG);
  multi(1,0,1,0);
  buttonCState[5] = digitalRead(SIG);
  multi(0,1,1,0);
  buttonCState[6] = digitalRead(SIG);
  multi(1,1,1,0);
  buttonCState[7] = digitalRead(SIG);
  multi(0,0,0,1);
  buttonCState[8] = digitalRead(SIG);
  multi(1,0,0,1);
  buttonCState[9] = digitalRead(SIG);
  multi(0,1,0,1);
  buttonCState[10] = digitalRead(SIG);
  multi(1,1,0,1);
  buttonCState[11] = digitalRead(SIG);
  multi(0,0,1,1);
  buttonCState[12] = digitalRead(SIG);
  multi(1,0,1,1);
  buttonCState[13] = digitalRead(SIG);
  multi(0,1,1,1);
  buttonCState[14] = digitalRead(SIG);
  multi(1,1,1,1);
  buttonCState[15] = digitalRead(SIG);
}

int btReturn(){  
  multi(0,0,0,0);
  if(digitalRead(SIG)){
    return 0;
  }
  multi(1,0,0,0);
  if(digitalRead(SIG)){
    return 1;
  }
  multi(0,1,0,0);
  if(digitalRead(SIG)){
    return 2;
  }
  multi(1,1,0,0);
  if(digitalRead(SIG)){
    return 3;
  }
  multi(0,0,1,0);
  if(digitalRead(SIG)){
    return 4;
  }
  multi(1,0,1,0);
  if(digitalRead(SIG)){
    return 5;
  }
  multi(0,1,1,0);
  if(digitalRead(SIG)){
    return 6;
  }
  multi(1,1,1,0);
  if(digitalRead(SIG)){
    return 7;
  }
  multi(0,0,0,1);
  if(digitalRead(SIG)){
    return 8;
  }
  multi(1,0,0,1);
  if(digitalRead(SIG)){
    return 9;
  }
  multi(0,1,0,1);
  if(digitalRead(SIG)){
    return 10;
  }
  multi(1,1,0,1);
  if(digitalRead(SIG)){
    return 11;
  }
  multi(0,0,1,1);
  if(digitalRead(SIG)){
    return 12;
  }
  multi(1,0,1,1);
  if(digitalRead(SIG)){
    return 13;
  }
  multi(0,1,1,1);
  if(digitalRead(SIG)){
    return 14;
  }
  multi(1,1,1,1);
  if(digitalRead(SIG)){
    return 15;
  }
  return 16;
}

void disparoBotoes(){
  for (int i=0; i<NButtons; i++) {    
    if (buttonCState[i] != buttonPState[i]) {
      if(buttonCState[i] == HIGH) {     
        MIDI.sendNoteOn(note+i+(16*deslocamento), 127, midiCh); // envia NoteOn(nota, velocity, canal midi)
        //MIDI.sendNoteOn(note[i], 127, midiCh); // use esta se quiser uma escala personalizada
        buttonPState[i] = buttonCState[i];
      }
      else {
        MIDI.sendNoteOn(note+i+(16*deslocamento), 0, midiCh); // use esta se quiser uma escala cromatica
        //MIDI.sendNoteOn(note[i], 0, midiCh); // use esta se quiser uma escala personalizada
        buttonPState[i] = buttonCState[i];
      }
    }    
  }
}

void noteOFF(){
  for (int i=0; i<NButtons; i++){  
    MIDI.sendNoteOn(note+i+(16*deslocamento), 0, midiCh);
  }
}

void disparoPotenciometros(){

  for (int i=0; i<NPots; i++) { // le todas entradas analogicas utilizadas
    potCState[i] = analogRead(pot[i]);
  }
  
  for (int i=0; i<NPots; i++) {

    potVar = abs(potCState[i] - potPState[i]); // calcula a variacao da porta analogica

    if (potVar >= varThreshold) {  //sets a threshold for the variance in the pot state, if it varies more than x it sends the cc message
      pTime[i] = millis(); // armazena o tempo previo
    }
    timer[i] = millis() - pTime[i]; // reseta o timer
    if (timer[i] < TIMEOUT) { // se o timer for menor que o tempo maximo permitido significa que o potenciometro ainda esta se movendo
      potMoving = true;
    }
    else {
      potMoving = false;
    }

    if (potMoving == true) { // se o potenciometro ainda esta se movendo, mande o control change
      MIDI.sendControlChange(cc+i, map(potCState[i], 0, 1023, 0, 127), midiCh); // envia Control Change (numero do CC, valor do CC, canal midi)
      potPState[i] = potCState[i]; // armazena a leitura atual do potenciometro para comparar com a proxima
    }
  }
}
