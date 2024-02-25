/*
https://github.com/ftjuh/I2Cwrapper
https://github.com/Koepel/How-to-use-the-Arduino-Wire-library/wiki/How-to-make-a-reliable-I2C-bus

 last update 28-01-2024
 */
#include <avr/power.h>
#include "Wire.h"

/*
* LAB Name: Arduino I2C Slave(RxTx)

*/
#include <Wire.h>

typedef int bob;

#define NUMBER_OF_BYTES 10

typedef struct I2CReg {
  bob buffer[NUMBER_OF_BYTES];
} I2CReg_t;



bob indexArray[] = {0x1B, 0x2B,0x3B,0x4B, 0x5B};

volatile bob reqArray[]  = {-1, -1,-1,-1, -1, -1, -1, -1 , -1};

volatile I2CReg_t regArray[sizeof(indexArray) / sizeof(bob)];

int tester(int regId) {
  
  int _regId = -1;
  
  for (int i = 0; i< sizeof(indexArray) / sizeof(bob); i++) {
    if (indexArray[i] == regId) {
      _regId= i;
    }
  }
  
  if ( _regId != -1 ) {

  }
  return _regId;
}
 
volatile byte RxByte;
volatile byte RegId = 0xFF;

volatile unsigned long rxTime = 0;
volatile unsigned long txTime = 0;

volatile byte zombieNumBytes = 0;

volatile byte zombieValue = 7;
 
void I2C_RxHandler(int numBytes)
{
  int bytesRecv = numBytes;
  byte idx = 0 ;

  int rowIndex = 0;

  //while(Wire.available()) {  // Read Any Received Data
  while (numBytes > 0) {
    RxByte = Wire.read();
    //Serial.print("[");Serial.print(RxByte);Serial.print("]");
    if (idx == 0) {  
      rowIndex = tester(RxByte);
      RegId = (byte) rowIndex;
      if (rowIndex != -1) {
        zombieNumBytes = 0;
      } else {
        zombieNumBytes = numBytes;
      }
    }
    //Serial.print("-");Serial.print(rowIndex);Serial.print("-");
    if ( bytesRecv > 1) {
      if (idx == 0) {         
         if (rowIndex != -1) {
          //Serial.print("+");
          regArray[rowIndex].buffer[0] = RxByte;
          regArray[rowIndex].buffer[NUMBER_OF_BYTES-1] = numBytes; // number of byte requested
         }
      } else if (rowIndex != -1 ) {
        //Serial.print("=");
        if (idx < sizeof(regArray[rowIndex].buffer) / sizeof(bob) ) {
          //Serial.print("#");
          regArray[rowIndex].buffer[idx] = RxByte;
        } 
        /*
        else {
          Serial.print("&");     
        }
        */
      }
    }
    idx++;
    numBytes --;
  }
  rxTime = millis();
}


 // onRequest(I2C_TxHandler);
void I2C_TxHandler(void)
{
  int Tx = -1;
  if ( RegId != 0xFF) {
    for ( int i=0 ; i < regArray[RegId].buffer[NUMBER_OF_BYTES-1]; i++) {
      Tx = (byte)regArray[RegId].buffer[i];
      Wire.write(Tx);   // skip first position, this is reg-no.   
      //Serial.print("|");Serial.print(Tx);Serial.print("|");  
    }     
  } else {
    byte xx[] = {47,11, 87, 22 };
    for (int i=0; i< zombieNumBytes; i++) {
      Tx = (byte)xx[i];
      Tx = zombieValue ++ ;
      Wire.write(Tx);  
      //Serial.print("!");Serial.print(Tx);Serial.print("!");     
    }
  }
 
  if (Tx == -1) { // request, but none is returned
    Wire.write(129);
    //Serial.print("*");
  }
  txTime = millis();
}

// #define GYRO_CONFIG 0x1B
// const int MPU_addr = 0x68; // I2C address of the MPU-6050
// I2C address = 55 for testing
 
void setup() {

  for (int i=0; i< sizeof( regArray) / sizeof (I2CReg_t) ; i++) {   
    regArray[i].buffer[0] = indexArray[i];

  }

  Wire.begin(0x55); // Initialize I2C (Slave Mode: address=0x55 )
  
  Wire.onReceive(I2C_RxHandler);
  Wire.onRequest(I2C_TxHandler);

  Serial.begin(115200);
  while (!Serial) { }

  Serial.println("I2C Slave MPU6050 Sim.");
  
  // testint x =-1;  byte u = (byte) x;  Serial.print(u);
  
}

unsigned long lastRxTime = 0;
unsigned long lastTxTime = 0;


void dump() {
  
    Serial.print("Data=");
    Serial.print(rxTime);Serial.print(", ");  Serial.print(txTime);Serial.print(", ");
    Serial.print(RegId);Serial.print(", ");
    Serial.println("");
    for (int i=0; i< sizeof( regArray) / sizeof (I2CReg_t) ; i++) {
      for (int j=0; j < sizeof(regArray[i].buffer) / sizeof(bob) ; j ++ ) {
        Serial.print(regArray[i].buffer[j]);Serial.print(", "); 
      }      
      Serial.println("");
    }
    Serial.println("");
    for (int i=0; i< 7; i++) {
       Serial.print(reqArray[i]);Serial.print(", "); 
    }
    Serial.println("");
}

 
void loop() {  
  if (lastRxTime != rxTime  ) {
    dump();
    lastRxTime = rxTime;
  } else {  
    if (lastTxTime != txTime ) {
      dump();
      lastTxTime = txTime;  
    }  
  }
}
