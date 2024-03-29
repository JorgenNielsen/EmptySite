
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#include <wiringx.h>

#define I2C_DEV "/dev/i2c-1"

#define I2C_ADDR_RAM	0x50  // RAM

#define I2C_ADDR_SIM	0x55 // GYRO

#define GYRO_CONFIG 0x1B

typedef int bob;

#define NUMBER_OF_BYTES 10

typedef struct I2CReg {
  bob buffer[NUMBER_OF_BYTES];
 } I2CReg_t;

I2CReg_t regArray[10];

bob indexArray[5] = {0x1B, 0x2B,0x3B,0x4B, 0x5B};

void tester(int regId) {
	
	int _regId = -1;
	
	for (int i = 0; i< sizeof(indexArray) / sizeof(bob); i++) {
		if (indexArray[i] == regId) {
			_regId= i;
		}
	}
	
	if ( _regId != -1 ) {
		regArray[_regId].buffer[0] = 1;
		regArray[_regId].buffer[NUMBER_OF_BYTES-1] = 0; // number of byte requested
	}
	
}

// docker
// \\wsl$\docker-desktop-data\version-pack-data\community\docker\volumes\3f5443bbb64b41aa3a17656b2d48994da19d34123cb58df978181337bc1765e9\_data\jon-examples\EmptySite\I2Cex


// Function to write to EEPROOM
/*
void writeEEPROM(int address, byte val, int i2c_address)
{
  // Begin transmission to I2C EEPROM
  Wire.beginTransmission(i2c_address);
 
  // Send memory address as two 8-bit bytes
  Wire.write((int)(address >> 8));   // MSB
  Wire.write((int)(address & 0xFF)); // LSB
 
  // Send data to be stored
  Wire.write(val);
 
  // End the transmission
  Wire.endTransmission();
 
  // Add 5ms delay for EEPROM
  delay(5);
}
*/
 
// Function to read from EEPROM
uint8_t readEEPROM( int fd, int address)
{
  // Define byte for received data
	uint8_t rcvData = 0xFF;

	//wiringXI2CWrite(fd,(uint8_t)((int)(address >> 8)));
	//wiringXI2CWrite(fd,(uint8_t)((int)(address & 0xFF)));

	//printf("I2C MSB: %d\n", (int)(address >> 8));
	//printf("I2C LSB: %d\n", (int)(address & 0xFF));
 
 
	wiringXI2CWrite(fd, (int)(address >> 8));
	wiringXI2CWrite(fd, (int)(address & 0xFF));

	//wiringXI2CWrite(fd, (uint8_t)(address >> 8));
	//wiringXI2CWrite(fd, (uint8_t)(address & 0xFF));

	//rcvData =  wiringXI2CRead(fd);
	rcvData =  wiringXI2CRead(fd);
 
  // Return the data as function output
  return rcvData;
}

// Function to read from EEPROM
void dumpEEPROM( int fd, int startaddress, int len)
{
  // Define byte for received data
	uint8_t rcvData = 0xFF;
 
	wiringXI2CWrite(fd, (int)(startaddress >> 8));
	wiringXI2CWrite(fd, (int)(startaddress & 0xFF));

	for ( int i=0; i< len; i++) {
		 printf("I2C read: %d = %d\n", i, wiringXI2CRead(fd));
	}
	
 
}
 

//int main(void)
int main ( int argc, char **argv )
{
    int fd_i2c;
    int data = 0;

	int addr 	= I2C_ADDR_RAM;
	
	printf("argc %d\n", argc);
	for (int i=0; i< argc; i++) {
		printf("arg %d, %s\n",i, argv[i]);
		if (strcmp(	argv[i], "sim") == 0) {
			addr 	= I2C_ADDR_SIM;
		}
	}
	// EEEPROM
	// https://dronebotworkshop.com/eeprom-arduino/
	// https://github.com/milkv-duo/duo-examples/blob/main/i2c/bmp280_i2c/bmp280_i2c.c
	 /* Example code to talk to a BMP280 temperature and pressure sensor

		NOTE: Ensure the device is capable of being driven at 3.3v NOT 5v. The Duo
		GPIO (and therefore I2C) cannot be used at 5v.

		You will need to use a level shifter on the I2C lines if you want to run the
		board at 5v.

		Connections on Milk-V Duo board, other boards may vary.

		I2C1_SDA (on Duo this is GPIOC9 (pin 14))  -> SDA on BMP280 board
		I2C1_SCL (on Duo this is GPIOC10 (pin 15)) -> SCL on BMP280 board
		3.3v (pin 36) -> VCC on BMP280 board
		GND (pin 38)  -> GND on BMP280 board
	 */

    if(wiringXSetup("duo", NULL) == -1) {
        wiringXGC();
        return -1;
    }

	

    if ((fd_i2c = wiringXI2CSetup(I2C_DEV, addr)) <0) {
        printf("I2C Setup failed: %d\n", fd_i2c);
        wiringXGC();
        return -1;
    }

	printf("I2C Setup: addr=0x%x handle=%d\n",addr,  fd_i2c);
	
	if (addr == I2C_ADDR_RAM) {			
		dumpEEPROM(fd_i2c, 0, 127);
		
	} else {
		
	 	unsigned long commitId = 0;
		commitId = 0xdbe7fbb7; // 1. version of sim
		commitId = 0x225de63f;
		
		switch(commitId) {
			case 0xdbe7fbb7: {
				wiringXI2CWrite(fd_i2c, GYRO_CONFIG);
				uint8_t GCValp  = wiringXI2CRead(fd_i2c);
				//
				wiringXI2CWriteReg8(fd_i2c, GYRO_CONFIG, 19);
				
				wiringXI2CWrite(fd_i2c, GYRO_CONFIG);
				uint8_t GCValc  = wiringXI2CRead(fd_i2c);
				
				
				printf("I2C read: %d = %d\n", GCValp, GCValc);		
				
				break;
			}
			case 0x225de63f: {
				uint8_t GCValp = 0;
				uint8_t GCValc = 0;
				
				//wiringXI2CWrite(fd_i2c, GYRO_CONFIG); // send one byte
				
				// GCValp  = wiringXI2CRead(fd_i2c);
				
				int rc = 0;
				
				//rc = wiringXI2CWriteReg8(fd_i2c, GYRO_CONFIG, 19);
				//GCValc  = wiringXI2CRead(fd_i2c);
				
				/*
				rc = wiringXI2CWriteReg16(fd_i2c, GYRO_CONFIG, 0x0E0F);	
				printf("I2C write rc: %d\n", rc);					
				for (int i=0; i< 1; i++)  {
					GCValc  = wiringXI2CRead(fd_i2c);
					printf("(%d) I2C read: %d\n", i, GCValc);
				}
				*/
				
				//wiringXI2CWrite(fd_i2c, GYRO_CONFIG);
				// GCValc  = wiringXI2CRead(fd_i2c);
				
				
				printf("I2C read: %d = %d\n", GCValp, GCValc);					
				
				dumpEEPROM(fd_i2c, 0, 12);
				break;
			}
		}
		
	}
	
	/*
	for ( int i=0; i< 128; i++) {
		 printf("I2C read: %d = %d\n", i, readEEPROM(fd_i2c, i));
	}
	*/
	
	//int startaddress = 64;	
	//wiringXI2CWrite(fd_i2c, (int)(startaddress >> 8));
	//wiringXI2CWrite(fd_i2c, (int)(startaddress & 0xFF));
	//wiringXI2CWrite(fd_i2c, 188);


	
	/*
	

	*/

}