#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>



#define in1 4
#define in2 5
#define in3 6
#define in4 10
#define enA 3
#define enB 9

long accelX, accelY, accelZ;
float gForceX, gForceY, gForceZ,angle;

enum MOVEMENT {FORWARD, BACKWARD, LEFT, RIGHT, STOP};

RF24 radio(7, 8); // CE, CSN
const byte address[6] = "00001";
 
float pitch, roll;
int flexI,flexD;
float data[4];

//MOVEMENT LEFT AND RIGHT SPEED VARIABLES
int RIGHT_A=100;
int RIGHT_B=90;
int LEFT_A=90;
int LEFT_B=100;

//SPEED VARIABLE
int defaultSpeed = 170;
int speedI = 0; //Speed Increment
int speedD = 0; //Speed Decrement
int speedC = 0; //Current Speed
int upper = 255-defaultSpeed; // Upper Range
int lower = defaultSpeed; // Lower Range

void setup()
{
  Serial.begin(9600);
    Wire.begin();
  
  setupMPU();
 
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();

  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  analogWrite(enA, 255);
  analogWrite(enB, 255);

 
}

void loop()
{
 //recordAccelRegisters();
//angle = 180 * atan (gForceX/sqrt(gForceY*gForceY + gForceZ*gForceZ))/M_PI;
//Serial.print(angle);
//Serial.print("................");
  if(radio.available())
  {
   radio.read(&data, sizeof(data));
   pitch=data[0];
   roll=data[1];
   flexI=data[2];
   flexD=data[3];
  }
  Serial.print(pitch);
  Serial.print(" ");
  Serial.print(roll);
  Serial.print(" ........ ");
  Serial.print(flexI);
  Serial.print("    ");
  Serial.print(flexD);
  Serial.print("...............");
  
  
  calculateSpeedC();
  calculateMovement();
  
  
  delay(100);
}

void calculateSpeedC()
{
  speedI=map(flexI,0,255,0,upper);
  speedD=map(flexD,0,255,lower,0);
  
  if(angle<= -15)
  speedC=220-speedD;
  else if(angle >= 15)
  speedC=100+speedI;
  else
  speedC=defaultSpeed+speedI-speedD;
  
  if(speedC>255)
  {
   speedC = 255;
  }
  else if(speedC<0)
  {
   speedC = 0;
  }
  Serial.println(speedC);
}

void calculateMovement()
{ 
 if(pitch<-30)
 {
  movement(FORWARD);
  analogWrite(enA, speedC);
  analogWrite(enB, speedC);
  
 }
 else if(pitch>30)
 {
  movement(BACKWARD);
  analogWrite(enA, speedC);
  analogWrite(enB, speedC);
 }
  
  if(roll>30)
 {
  movement(RIGHT);
  analogWrite(enA, RIGHT_A);
  analogWrite(enB, RIGHT_B);
 }
 else if(roll<-30)
 {
  movement(LEFT);
  analogWrite(enA, LEFT_A);
  analogWrite(enB, LEFT_B);
 }
 
 if(pitch<30 && pitch>-30 && roll<30 && roll>-30)
 {
  movement(STOP);
 }
}

void movement(MOVEMENT dir)
{
  switch (dir)
  {
    case FORWARD:
      digitalWrite(in1, HIGH);
      digitalWrite(in2, LOW);
      digitalWrite(in3, HIGH);
      digitalWrite(in4, LOW);
      break;

    case BACKWARD:
      digitalWrite(in1, LOW);
      digitalWrite(in2, HIGH);
      digitalWrite(in3, LOW);
      digitalWrite(in4, HIGH);
      break;

    case LEFT:
      digitalWrite(in1, LOW);
      digitalWrite(in2, HIGH);
      digitalWrite(in3, HIGH);
      digitalWrite(in4, LOW);
      break;

    case RIGHT:
      digitalWrite(in1, HIGH);
      digitalWrite(in2, LOW);
      digitalWrite(in3, LOW);
      digitalWrite(in4, HIGH);
      break;

    case STOP:
      digitalWrite(in1, LOW);
      digitalWrite(in2, LOW);
      digitalWrite(in3, LOW);
      digitalWrite(in4, LOW);
      break;
  }
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
