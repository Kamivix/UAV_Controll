
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

//Deklaracja serw
Servo channel1;
Servo channel2;
Servo channel3;
Servo channel4;
//Zmienne reprezentujące szerokość impulsu
int width_1 = 0;
int width_2 = 0;
int width_3 = 0;
int width_4 = 0;
//Deklaracja struktury do odbioru danych
struct Signal {
byte throttle;      
byte pitch;
byte roll;
byte yaw;
};
//Deklaracja obektu struktury, inicjalizacja modułu odbiorczego
Signal dataReceived;
const byte address[6] = "00001";
RF24 radio(7, 8); 

unsigned long lastRecvTime = 0;
void setup()
{

   
//Przypisanie obeiktów serw do odpowiednich pinów
  channel1.attach(6);
  channel2.attach(5);
  channel3.attach(3);
  channel4.attach(9);
  
  //Konfiguracja modułu odbiorczego
  ResetData();
  radio.begin();
  radio.openReadingPipe(0,address);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_HIGH);
  radio.startListening();
  
}
void loop()
{
recvData();
unsigned long now = millis();
//Reset jeżeli utracono połączenie
if ( now - lastRecvTime > 1000 ) {
ResetData(); // Signal lost.. Reset data 
}

//Mapowanie danych odebranych na mikrosekundy
width_4 = map(dataReceived.yaw,      0, 255, 1000, 2000);     
width_2 = map(dataReceived.pitch,    0, 255, 1000, 2000);    
width_3 = map(dataReceived.throttle, 0, 255, 1000, 2000);    
width_1 = map(dataReceived.roll,     0, 255, 1000, 2000);    
//Wysłanie danych do kontrolera lotu
channel1.writeMicroseconds(width_1);
channel2.writeMicroseconds(width_2);
channel3.writeMicroseconds(width_3);
channel4.writeMicroseconds(width_4);
}

//Funckja obsługująca odbiór danych
void recvData()
{
while ( radio.available() ) {
radio.read(&dataReceived, sizeof(Signal));
lastRecvTime = millis();   
}
}
//Funkcja resetująca dane
void ResetData()
{
dataReceived.roll =127 ;    
dataReceived.pitch = 127;   
dataReceived.throttle = 12; 
dataReceived.yaw = 127;   
}
