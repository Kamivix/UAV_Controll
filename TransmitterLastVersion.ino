
  #include <nRF24L01.h>
  #include <RF24.h>
  #include <Wire.h>
  //Inicjalizacja zmiennych do obsługi MPU6050 oraz drążka
  const int mpuAddress = 0x68; 
float accXAxis, accYAxis, accZAxis,gyroxAxis, gyroYAxis;
float accAngleinXAxis, accAngleinYAxis, gyroAngleInXAxis, gyroAngleInYAxis;
float ErrorInAccX, ErrorInAccY, ErrorInGyroX, ErrorInGyroY,elapsedTime, timeNow, timeBefore;
float roll, pitch, yaw;
//Inicjalizacja modułu nadawczego
const byte addressToCommunication[6] = "00001";
  RF24 radio(7, 8); 
  //Struktura danych wysyłana do odbiornika
  struct dataToSend {
  byte throttle;
  byte pitch;
  byte roll;
  byte yaw;
};
  dataToSend data;
 
  void setup()
{
  //Obsługa modułu MPU6050 do odczytu położenia ręki
  Wire.begin();                      
  Wire.beginTransmission(mpuAddress);     // Start communication with mpu6050 
  Wire.write(0x6B);                       // communication with register 6B
  Wire.write(0x00);                       // Reset
  Wire.endTransmission(true);        
 
  //Konfiguracja modułu nadawczego
  radio.begin();
  radio.openWritingPipe(addressToCommunication);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_HIGH);
  radio.stopListening(); 
  clearData();
 
}
  



  void loop()
{
   //Odczyt danych z akcelerometru używając magistrali I2C
  Wire.beginTransmission(mpuAddress);
  Wire.write(0x3B); 
  Wire.endTransmission(false);
  Wire.requestFrom(mpuAddress, 6, true); 

  accXAxis = (Wire.read() << 8 | Wire.read()) / 16384.0; // X-axis 
  accYAxis = (Wire.read() << 8 | Wire.read()) / 16384.0; // Y-axis 
  accZAxis = (Wire.read() << 8 | Wire.read()) / 16384.0; // Z-axis 
 
// Obliczenie wskazań akcelerometru dla osi x i y
  accAngleinXAxis = (atan(accYAxis / sqrt(pow(accXAxis, 2) + pow(accZAxis, 2))) * 180 / PI) - 0.58; 
  accAngleinYAxis = (atan(-1 * accXAxis / sqrt(pow(accYAxis, 2) + pow(accZAxis, 2))) * 180 / PI) + 1.58; 
// Odczyt danych żyroskopu używając magistrali I2C
  timeBefore = timeNow;        // 
  timeNow = millis();            
  elapsedTime = (timeNow - timeBefore) / 1000; 
  Wire.beginTransmission(mpuAddress);
  Wire.write(0x43);
  Wire.endTransmission(false);
  Wire.requestFrom(mpuAddress, 4, true); 
  gyroxAxis = (Wire.read() << 8 | Wire.read()) / 131.0; 
  gyroYAxis = (Wire.read() << 8 | Wire.read()) / 131.0;
 
  gyroAngleInXAxis = gyroAngleInXAxis + gyroxAxis * elapsedTime; // deg/s * s = deg
  gyroAngleInYAxis = gyroAngleInYAxis + gyroYAxis * elapsedTime;
// Obliczenie zmiennych pitch oraz roll
 gyroAngleInXAxis = 0.96 * gyroAngleInXAxis + 0.041 * accAngleinXAxis;
gyroAngleInYAxis = 0.96 * gyroAngleInYAxis + 0.041 * accAngleinYAxis;
roll = gyroAngleInXAxis;
pitch = gyroAngleInYAxis;

// Mapowanie zmiennych na dane akceptowane przez odbiornik
  roll=map(roll,-100,100,0,255);
  roll=constrain(roll,0,255);
  pitch=map(pitch,-100,100,0,255);
  pitch=constrain(pitch,0,255);

  data.throttle = joystickValues( analogRead(A0), 12, 524, 1020, true );  //  
  data.yaw = joystickValues( analogRead(A1), 12, 500, 1020, true );    
// Ignorowanie drgań pilota w położeniu początkowym
   if(pitch>=117 && pitch<=137){
    data.roll=127;}
    else
 { data.roll = pitch;    } 
 if(roll>=117 && roll<=137)
 { data.pitch=127;
  }
  else{
  data.pitch = roll;    
  }
   
//Wysłanie danych
  radio.write(&data, sizeof(dataToSend));
}
//Funkcja czyszcząca dane
  void clearData() 
{
  data.throttle = 12;   
  data.roll = 127;     
  data.yaw = 127;      
}

//Funkcja poprawiajaca odczyt danych z drążka analogowego
  int joystickValues(int value, int low, int mid, int up, bool rev)
{
  value = constrain(value, low, up);
  if ( value < mid )
  value = map(value, low, mid, 0, 128);
  else
  value = map(value, mid, up, 128, 255);

  if(rev){
    return (255-value);}
    else
    {return value;
      }
}
