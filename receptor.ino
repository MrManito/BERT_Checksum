// Define la trama y los bloques que se pasan
byte* trama;
byte send_data[32];
// Parametros de trama
byte header_tail = 0b01111110;
byte types[4] = {0b00000000, 0b00000001,0b00000010,0b00000011};
// Velocidades y payloads
int speeds[5] = {19200,1200,4800,9600,19200};
int charges[4] = {1000,400,600,1000};
// Contadores para saber que velocidad o peso usar
byte speed_counter = 0b00000000;
byte charge_counter = 0b00000000;
// Banderas de control
bool handshake = true; // handshake o no
bool ack = false; // acknowlege
bool test_flag = true;
bool results = false;

bool prueba = false;


// Otras variables
int block_size = 32;
byte x[3];

//número de pruebas 
int tests = 93; 
//int tests = 88; 
//int tests = 83; 

//número de errores
int bert = 18; // 5% 
//int bert = 8; // 10% 
//int bert = 5; // 15% 

void setup() {
  Serial.begin(300);  // Configura la comunicación serial a 9600 baudios
}

void wait_for_ack(){
  
  delay(1500);
  
  if (Serial.available() != 0){
    if (Serial.read() == header_tail) {
      ack = true;
    }else{
      ack = false; 
    }
    Serial.readBytes(x,3);
  }
  
}

// Sumas de verificación
void checksum(int charge){
  // Calculo del checkum
  unsigned long sum = 0;
  for (int j = 1; j < charge + 2; j++) {
    sum += trama[j];
    //Serial.println(trama[j]);
  }
  trama[charge+2] = (sum >> 24) & 0xFF;
  trama[charge+3] = (sum >> 16) & 0xFF;
  trama[charge+4] = (sum >> 8) & 0xFF;
  trama[charge+5] = sum & 0xFF;
}

// Funcion que crea las trama
void create_plot(){
  if (handshake){
    delete trama;
    trama = new byte[11];
    trama[0] = header_tail;
    trama[1] = types[0];
    trama[2] = highByte(speeds[speed_counter]);
    trama[3] = lowByte(speeds[speed_counter]);
    trama[4] = highByte(charges[charge_counter]);
    trama[5] = lowByte(charges[charge_counter]);
    trama[10] = header_tail;
    checksum(4);
  }else if (results){
    delete trama;
    trama = new byte[charges[charge_counter]+7];
    trama[0] = header_tail;
    trama[1] = types[3];
    for (int i = 2; i < charges[charge_counter] + 2 ; i++) {
      	trama[i] = 0;
    }
    checksum(charges[charge_counter]);
    trama[charges[charge_counter] + 6] = header_tail;
  }
  else{
    delete trama;
    trama = new byte[charges[charge_counter]+7];
    trama[0] = header_tail;
    trama[1] = types[1];
    for (int i = 2; i < charges[charge_counter] + 2 ; i++) {
        byte number = random(255);
        while(number == 0b01111110){
          number = random(255);
        }
        trama[i] = number;
      	//trama[i] = 97;
      	
    }
    checksum(charges[charge_counter]);
    trama[charges[charge_counter] + 6] = header_tail;
  }
}

// Envia la trama
void send_plot(bool prueba) {
 if (handshake){
   for(int i = 0; i < 11; i++){
     int x = Serial.write(trama[i]);
     delay(10);
   }
 }else{
   for(int i = 0; i < charges[charge_counter]+7; i++){
     if((i == 5 or i == 10) and prueba){
       Serial.write('2');
     }else{
       Serial.write(trama[i]);
     }
     delay(10);
   }  
 }
}

void loop() {
  delay(1000);
  if (test_flag){
    if (handshake){
      // Crea la trama de handshake
      create_plot();
      
      while(!ack){
        send_plot(false);
        wait_for_ack(); 
        //Serial.println("aca");
      }
      
      delay(1000); //Cambia de velocidad
      Serial.end();
      Serial.begin(speeds[speed_counter]);
      ack = false;
      handshake = false;
    }
    else{
      int i = 0;
      for (i; i < tests; i++) {
        // Crea la trama de payload
        create_plot();
        while(!ack){
          if(i % bert == 0 and prueba){
            send_plot(true);
            prueba = false;
          }else{
            send_plot(false);
          }
         
          wait_for_ack(); 
        }
        ack = false;
        prueba = true;
      }
      test_flag = false;
      
      
      results = true;
      // TRAMA RESULTADOS FINALES
      create_plot();
      send_plot(false);

      
      
      //if(charge_counter < 4){
      // 	charge_counter++;
      //}else{
      //  charge_counter = 0;
      //  speed_counter++;
      //}

      //handshake = true;
        
      //if(speed_counter > 4){
      //  while(true){}//kill
      //}

      while(true){}//kill
    }
  }
}