/**
 * Copyright (c) 2023 Milk-V
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **/

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

#include <time.h>
#include <string.h>


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>



#include <wiringx.h>

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

// depends on which port to use
#define I2C_DEV "/dev/i2c-1"

// device has default bus address of 0x76
#define I2C_ADDR  0x76

// hardware registers
#define REG_CONFIG    0xF5
#define REG_CTRL_MEAS 0xF4
#define REG_RESET     0xE0

#define REG_TEMP_XLSB 0xFC
#define REG_TEMP_LSB  0xFB
#define REG_TEMP_MSB  0xFA

#define REG_PRESSURE_XLSB 0xF9
#define REG_PRESSURE_LSB  0xF8
#define REG_PRESSURE_MSB  0xF7

// calibration registers
#define REG_DIG_T1 0x88
#define REG_DIG_T2 0x8A
#define REG_DIG_T3 0x8C

#define REG_DIG_P1 0x8E
#define REG_DIG_P2 0x90
#define REG_DIG_P3 0x92
#define REG_DIG_P4 0x94
#define REG_DIG_P5 0x96
#define REG_DIG_P6 0x98
#define REG_DIG_P7 0x9A
#define REG_DIG_P8 0x9C
#define REG_DIG_P9 0x9E

/*
* Immutable calibration data read from bmp280
*/
struct bmp280_calib_param {
    // temperature params
    uint16_t dig_t1;
    int16_t dig_t2;
    int16_t dig_t3;

    // pressure params
    uint16_t dig_p1;
    int16_t dig_p2;
    int16_t dig_p3;
    int16_t dig_p4;
    int16_t dig_p5;
    int16_t dig_p6;
    int16_t dig_p7;
    int16_t dig_p8;
    int16_t dig_p9;
};

void ntpdate() {
	char    *hostname="pool.ntp.org";
	int portno=123;     //NTP is port 123
	int maxlen=1024;        //check our buffers
	int i;          // misc var i
	unsigned char msg[48]={010,0,0,0,0,0,0,0,0};    // the packet we send
	unsigned long  buf[maxlen]; // the buffer we get back
	//struct in_addr ipaddr;        //  
	struct protoent *proto;     //
	struct sockaddr_in server_addr;
	int s;  // socket
	long tmit;   // the time -- This is a time_t sort of

	//use Socket;
	//
	//#we use the system call to open a UDP socket
	//socket(SOCKET, PF_INET, SOCK_DGRAM, getprotobyname("udp")) or die "socket: $!";
	proto=getprotobyname("udp");
	s=socket(PF_INET, SOCK_DGRAM, proto->p_proto);
	if(s) {
		perror("asd");
		printf("socket=%d\n",s);
	}
	//
	//#convert hostname to ipaddress if needed
	//$ipaddr   = inet_aton($HOSTNAME);
	memset( &server_addr, 0, sizeof( server_addr ));
	server_addr.sin_family=AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(hostname);
	//argv[1] );
	//i   = inet_aton(hostname,&server_addr.sin_addr);
	server_addr.sin_port=htons(portno);
	//printf("ipaddr (in hex): %x\n",server_addr.sin_addr);

	/*
	 * build a message.  Our message is all zeros except for a one in the
	 * protocol version field
	 * msg[] in binary is 00 001 000 00000000 
	 * it should be a total of 48 bytes long
	*/

	// send the data
	printf("sending data..\n");
	i=sendto(s,msg,sizeof(msg),0,(struct sockaddr *)&server_addr,sizeof(server_addr));

	// get the data back
	i=recv(s,buf,sizeof(buf),0);
	printf("recvfr: %d\n",i);
	//perror("recvfr:");

	//We get 12 long words back in Network order
	/*
	for(i=0;i<12;i++)
		printf("%d\t%-8x\n",i,ntohl(buf[i]));
	*/

	/*
	 * The high word of transmit time is the 10th word we get back
	 * tmit is the time in seconds not accounting for network delays which
	 * should be way less than a second if this is a local NTP server
	 */

	tmit=ntohl((time_t)buf[10]);    //# get transmit time
	//printf("tmit=%d\n",tmit);

	/*
	 * Convert time to unix standard time NTP is number of seconds since 0000
	 * UT on 1 January 1900 unix time is seconds since 0000 UT on 1 January
	 * 1970 There has been a trend to add a 2 leap seconds every 3 years.
	 * Leap seconds are only an issue the last second of the month in June and
	 * December if you don't try to set the clock then it can be ignored but
	 * this is importaint to people who coordinate times with GPS clock sources.
	 */

	tmit-= 2208988800U; 
	//printf("tmit=%d\n",tmit);
	/* use unix library function to show me the local time (it takes care
	 * of timezone issues for both north and south of the equator and places
	 * that do Summer time/ Daylight savings time.
	 */


	//#compare to system time
	printf("Time: %s",ctime(&tmit));
	i=time(0);
	//printf("%d-%d=%d\n",i,tmit,i-tmit);
	printf("System time is %d seconds off\n",i-tmit);
}

char * gettime()
{
    time_t t;
    struct tm *info;
    static char time_str[10]; 

    time(&t);
    info = localtime(&t);
    sprintf(time_str,"%02d:%02d:%02d",(*info).tm_hour, (*info).tm_min, (*info).tm_sec);

    return time_str;
}

void bmp280_read_raw(int fd, int32_t* temp, int32_t* pressure)
{
    // BMP280 data registers are auto-incrementing and we have 3 temperature and
    // pressure registers each, so we start at 0xF7 and read 6 bytes to 0xFC
    // note: normal mode does not require further ctrl_meas and config register writes

    uint8_t buf[9];

    wiringXI2CWrite(fd, REG_PRESSURE_MSB);
    buf[0] = wiringXI2CRead(fd);
    buf[1] = wiringXI2CRead(fd);
    buf[2] = wiringXI2CRead(fd);

    buf[3] = wiringXI2CRead(fd);
    buf[4] = wiringXI2CRead(fd);
    buf[5] = wiringXI2CRead(fd);

    // store the 20 bit read in a 32 bit signed integer for conversion
    *pressure = (buf[0] << 12) | (buf[1] << 4) | (buf[2] >> 4);
    *temp = (buf[3] << 12) | (buf[4] << 4) | (buf[5] >> 4);
}

void bmp280_init(int fd) {
    // use the "handheld device dynamic" optimal setting (see datasheet)

    // 500ms sampling time, x16 filter
    const uint8_t reg_config_val = ((0x04 << 5) | (0x05 << 2)) & 0xFC;
    wiringXI2CWriteReg8(fd, REG_CONFIG, reg_config_val);

    // osrs_t x1, osrs_p x4, normal mode operation
    const uint8_t reg_ctrl_meas_val = (0x01 << 5) | (0x03 << 2) | (0x03);
    wiringXI2CWriteReg8(fd, REG_CTRL_MEAS, reg_ctrl_meas_val);
}

void bmp280_get_calib_params(int fd, struct bmp280_calib_param* params) {
    // raw temp and pressure values need to be calibrated according to
    // parameters generated during the manufacturing of the sensor
    // there are 3 temperature params, and 9 pressure params

    params->dig_t1 = (uint16_t)wiringXI2CReadReg16(fd, REG_DIG_T1);
    params->dig_t2 = (int16_t)wiringXI2CReadReg16(fd, REG_DIG_T2);
    params->dig_t3 = (int16_t)wiringXI2CReadReg16(fd, REG_DIG_T3);

    params->dig_p1 = (uint16_t)wiringXI2CReadReg16(fd, REG_DIG_P1);
    params->dig_p2 = (int16_t)wiringXI2CReadReg16(fd, REG_DIG_P2);
    params->dig_p3 = (int16_t)wiringXI2CReadReg16(fd, REG_DIG_P3);
    params->dig_p4 = (int16_t)wiringXI2CReadReg16(fd, REG_DIG_P4);
    params->dig_p5 = (int16_t)wiringXI2CReadReg16(fd, REG_DIG_P5);
    params->dig_p6 = (int16_t)wiringXI2CReadReg16(fd, REG_DIG_P6);
    params->dig_p7 = (int16_t)wiringXI2CReadReg16(fd, REG_DIG_P7);
    params->dig_p8 = (int16_t)wiringXI2CReadReg16(fd, REG_DIG_P8);
    params->dig_p9 = (int16_t)wiringXI2CReadReg16(fd, REG_DIG_P9);
}

// intermediate function that calculates the fine resolution temperature
// used for both pressure and temperature conversions
int32_t bmp280_convert(int32_t temp, struct bmp280_calib_param* params) {
    // use the 32-bit fixed point compensation implementation given in the
    // datasheet

    int32_t var1, var2;
    var1 = ((((temp >> 3) - ((int32_t)params->dig_t1 << 1))) * ((int32_t)params->dig_t2)) >> 11;
    var2 = (((((temp >> 4) - ((int32_t)params->dig_t1)) * ((temp >> 4) - ((int32_t)params->dig_t1))) >> 12) * ((int32_t)params->dig_t3)) >> 14;
    return var1 + var2;
}

int32_t bmp280_convert_temp(int32_t temp, struct bmp280_calib_param* params) {
    // uses the BMP280 calibration parameters to compensate the temperature value read from its registers
    int32_t t_fine = bmp280_convert(temp, params);
    return (t_fine * 5 + 128) >> 8;
}

int32_t bmp280_convert_pressure(int32_t pressure, int32_t temp, struct bmp280_calib_param* params) {
    // uses the BMP280 calibration parameters to compensate the pressure value read from its registers

    int32_t t_fine = bmp280_convert(temp, params);

    int32_t var1, var2;
    uint32_t converted = 0.0;
    var1 = (((int32_t)t_fine) >> 1) - (int32_t)64000;
    var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((int32_t)params->dig_p6);
    var2 += ((var1 * ((int32_t)params->dig_p5)) << 1);
    var2 = (var2 >> 2) + (((int32_t)params->dig_p4) << 16);
    var1 = (((params->dig_p3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + ((((int32_t)params->dig_p2) * var1) >> 1)) >> 18;
    var1 = ((((32768 + var1)) * ((int32_t)params->dig_p1)) >> 15);
    if (var1 == 0) {
        return 0;  // avoid exception caused by division by zero
    }
    converted = (((uint32_t)(((int32_t)1048576) - pressure) - (var2 >> 12))) * 3125;
    if (converted < 0x80000000) {
        converted = (converted << 1) / ((uint32_t)var1);
    } else {
        converted = (converted / (uint32_t)var1) * 2;
    }
    var1 = (((int32_t)params->dig_p9) * ((int32_t)(((converted >> 3) * (converted >> 3)) >> 13))) >> 12;
    var2 = (((int32_t)(converted >> 2)) * ((int32_t)params->dig_p8)) >> 13;
    converted = (uint32_t)((int32_t)converted + ((var1 + var2 + params->dig_p7) >> 4));
    return converted;
}

int main() {
    int fd_i2c;
    int data = 0;

	//ntpdate();

    if(wiringXSetup("duo", NULL) == -1) {
        wiringXGC();
        return -1;
    }

    if ((fd_i2c = wiringXI2CSetup(I2C_DEV, I2C_ADDR)) <0) {
        printf("I2C Setup failed: %i\n", fd_i2c);
        return -1;
    }

    // configure BMP280
    bmp280_init(fd_i2c);

    // retrieve fixed compensation params
    struct bmp280_calib_param params;
    bmp280_get_calib_params(fd_i2c, &params);

    usleep(250000); // sleep so that data polling and register update don't collide

    int32_t raw_temperature;
    int32_t raw_pressure;

    while (1) {
        bmp280_read_raw(fd_i2c, &raw_temperature, &raw_pressure);
        int32_t temperature = bmp280_convert_temp(raw_temperature, &params);
        int32_t pressure = bmp280_convert_pressure(raw_pressure, raw_temperature, &params);
		printf("%s: ", gettime());
        printf("Temp. = %.2f C, ", temperature / 100.f);
        printf("Pressure = %.3f kPa\n", pressure / 1000.f);
        // poll every 1s
        sleep(1);
    }
}
