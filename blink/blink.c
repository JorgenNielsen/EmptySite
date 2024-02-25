#include <stdio.h>
#include <unistd.h>

#include <wiringx.h>

int main() {
    int DUO_LED = 25; // GP24
	
	int i2c_DAT = 14; // GP14
	int i2c_Clk = 15; // GP15

    if(wiringXSetup("duo", NULL) == -1) {
        wiringXGC();
        return -1;
    }

    if(wiringXValidGPIO(DUO_LED) != 0) {
        printf("Invalid GPIO %d\n", DUO_LED);
    }

    //pinMode(DUO_LED, PINMODE_OUTPUT);
	pinMode(i2c_DAT, PINMODE_OUTPUT);
	pinMode(i2c_Clk, PINMODE_OUTPUT);

	// I2C pin test
    while(1) {
        printf("Duo LED GPIO (wiringX) %d: High\n", i2c_DAT);
        digitalWrite(i2c_DAT, HIGH);
        sleep(0.1);
        printf("Duo LED GPIO (wiringX) %d: Low\n", i2c_DAT);
        digitalWrite(i2c_DAT, LOW);
        sleep(0.1);
		
		printf("Duo LED GPIO (wiringX) %d: High\n", i2c_Clk);
        digitalWrite(i2c_Clk, HIGH);
        sleep(0.1);
        printf("Duo LED GPIO (wiringX) %d: Low\n", i2c_Clk);
        digitalWrite(i2c_Clk, LOW);
        sleep(0.1);
    }

    return 0;
}
