#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>

#define flexpinI A0
#define flexpinD A1


RF24 radio(7, 8); // CE, CSN
const byte address[6] = "00001";

int flexI,flexD;
long accelX, accelY, accelZ;
float gForceX, gForceY, gForceZ;
float data[4] = {0}; // 0 = pitch, 1 = roll, 2 = flexI, 3 = flexD 

void setup()
{

  Serial.begin(9600);
    
  Wire.begin();
  
  setupMPU();
  
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
}

void loop()
{ 
  recordAccelRegisters();

  flexI = analogRead(flexpinI);
  flexD = analogRead(flexpinD);
 Serial.print(flexI);
  Serial.print("       ");
 Serial.print(flexD);
 Serial.print("       ");
  
  data[0] = 180 * atan (gForceX/sqrt(gForceY*gForceY + gForceZ*gForceZ))/M_PI; //pitch
  data[1] = 180 * atan (gForceY/sqrt(gForceX*gForceX + gForceZ*gForceZ))/M_PI; //roll
  data[2] = map(flexI, 0, 530, 0, 255);
  data[3] = map(flexD, 240, 45, 255 , 0);
  Serial.print(data[2]);
  Serial.print("       ");
  Serial.print(data[3]);
  Serial.print("////////////////");
   Serial.print(data[0]);
   Serial.print(".............");
   Serial.println(data[1]);
  delay(100);
  radio.write(&data, sizeof(data));
  
}

void setupMPU()
{
  Wire.beginTransmission(0b1101000); //This is the I2C address of the MPU (b1101000/b1101001 for AC0 low/high datasheet sec. 9.2)
    
  Wire.write(0x6B); //Accessing the register 6B - Power Management (Sec. 4.28)
  Wire.write(0b00000000); //Setting SLEEP register to 0. (Required; see Note on p. 9)
   
  Wire.endTransmission();
    
  Wire.beginTransmission(0b1101000); //I2C address of the MPU
  Wire.write(0x1C); //Accessing the register 1C - Acccelerometer Configuration (Sec. 4.5)
  Wire.write(0b00000000); //Setting the accel to +/- 2g
   
  Wire.endTransmission();
}

void recordAccelRegisters()
{
   
  Wire.beginTransmission(0b1101000); //I2C address of the MPU

  Wire.write(0x3B); //Starting register for Accel Readings
 
  Wire.endTransmission();
  Wire.requestFrom(0b1101000, 6); //Request Accel Registers (3B - 40)
  
  while (Wire.available() < 6);
 
  accelX = Wire.read() << 8 | Wire.read(); //Store first two bytes into accelX
  accelY = Wire.read() << 8 | Wire.read(); //Store middle two bytes into accelY
  accelZ = Wire.read() << 8 | Wire.read(); //Store last two bytes into accelZ
 
  processAccelData();
}

void processAccelData()
{
  gForceX = accelX / 16384.0;
  gForceY = accelY / 16384.0;
  gForceZ = accelZ / 16384.0;
 
}
