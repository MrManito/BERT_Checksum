#include<LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 5, 4, 3, 6);
// Define la trama y los bloques que se pasan
byte* trama;
byte header_tail = 0b01111110;
byte types[4] = {0b00000000, 0b00000001,0b00000010,0b00000011};
int speed = 300;
int charge = 4;
int contador = 0;
int tramas_recibidas = 0;
int tramas_incorrectas = 0; 

// Banderas de control
bool handshake = true; 
bool error = false;

void setup() {
  Serial.begin(300); 
  lcd.begin(16, 2);  
  lcd.setCursor(0, 0);
  lcd.print("Inicializando...");
}

bool validate_checksum() {
  unsigned long sum = 0;
  for (int j = 1; j < charge + 2; j++) {
    sum += trama[j];
  }
  lcd.clear();
  lcd.print(sum);
  unsigned long receivedChecksum = ((unsigned long)trama[charge+2] << 24) | ((unsigned long)trama[charge+3] << 16) | ((unsigned long)trama[charge+4] << 8) | trama[charge+5];
  lcd.setCursor(0, 1);
  lcd.print(receivedChecksum);
  //delay(1000);
  return sum == receivedChecksum;
}

void send_ack(){
  byte ack[4] = {header_tail,types[2],types[2],header_tail};
  Serial.write(ack,4);
}

void read_plot(){
  trama[0] = Serial.read();
  //lcd.clear();
  //lcd.print(trama[0]);
  
  while (trama[0] != header_tail) {
    trama[0] = Serial.read();
    //lcd.clear();
    //lcd.print(trama[0]);
  }

  if (handshake) {
    for (int i = 1; i < 11; i++) {
      while (Serial.available() < 1) {}  // Esperar hasta que haya al menos un byte disponible
      trama[i] = Serial.read();
    }
    
    if(trama[10] != header_tail){
      tramas_incorrectas++;
      error = true;
    }
    
  } else {
    for (int i = 1; i < charge + 7; i++) {
      while (Serial.available() < 1) {}  // Esperar hasta que haya al menos un byte disponible
      trama[i] = Serial.read();
    }
    
    if(trama[charge + 6] != header_tail){
      tramas_incorrectas++;
      error = true;
    }
  }
}

void analyze_plot(){
  if (validate_checksum()){
    if (trama[1] == types[0]){
      speed = (trama[2] << 8) | trama[3];
      charge = (trama[4] << 8) | trama[5];

      lcd.clear();
      lcd.print("Speed: ");
      lcd.print(speed);
      lcd.setCursor(0, 1);
      lcd.print("Charge: ");
      lcd.print(charge);
      //delay(1000);
      lcd.clear();
      //delay(1000);
      send_ack();
      contador = 0;
      delay(1000);
      Serial.end();
      Serial.begin(speed);
      //delay(3000);
      handshake = false;
    }else if (trama[1] == types[3]){
      double porcentaje =  (static_cast<double>(tramas_incorrectas) / tramas_recibidas)*100;
  	  lcd.clear();
  	  lcd.print("Tasa Error: ");
      lcd.setCursor(0, 1);
      lcd.print(porcentaje);
      delay(3000);
      
      while(true){}
    }
    else{
      // TRAMAS NORMALES
      lcd.clear();
      lcd.print("Trama Procesada...");
      //delay(100);
      lcd.clear();
      send_ack();
      contador++;
    }
  }else{
     tramas_incorrectas++;   
  }
}

void loop() {
  delay(1000);
  lcd.clear();
  trama = new byte[(charge + 7)];
  read_plot();
  tramas_recibidas++;
  
  if(!error){
    analyze_plot();
  	lcd.print("Erroneas: ");
  	lcd.print(tramas_incorrectas);
  	lcd.setCursor(0, 1);
  	lcd.print("Enviadas: ");
  	lcd.print(tramas_recibidas);
  }else{
  	error = false;
    lcd.print("Fallo");
  }
  
  delete trama;
  delay(1000);
}