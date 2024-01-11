
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include <unistd.h>

#include <sys/stat.h>   // stat
#include <stdbool.h>    // bool type

#include <wiringx.h>

// git init
// create / change files
// git add .
// git commit -m "comment"
// git push --set-upstream https://github.com/JorgenNielsen/EmptySite master
// root@46e322a49825:/workspace# source ./tpu-mlir/envsetup.sh

// git clone https://github.com/JorgenNielsen/EmptySite  
// root@46e322a49825:/workspace/duo-examples#  source envsetup.sh
//
// cd /workspace/jon-examples/EmptySite/uart_test
// root@46e322a49825:/workspace/jon-examples/EmptySite/uart_test# git pull origin master
// make
// scp uart_test root@192.168.42.1:/root/


// start
/*
	PS C:\Users\JON.PC49> docker ps
	CONTAINER ID   IMAGE                  COMMAND       CREATED      STATUS         PORTS     NAMES
	46e322a49825   sophgo/tpuc_dev:v3.1   "/bin/bash"   2 days ago   Up 6 seconds             DuoTPU
	PS C:\Users\JON.PC49> docker exec -it 46e322a49825 /bin/bash
	root@46e322a49825:/workspace# ls
	duo-examples  jon-examples  tpu-mlir
	root@46e322a49825:/workspace# cd jon-examples/
	root@46e322a49825:/workspace/jon-examples# ls
	EmptySite  lys_demo
	root@46e322a49825:/workspace/jon-examples# cd EmptySite/
	root@46e322a49825:/workspace/jon-examples/EmptySite# ls
	readme.txt  uart_test
	root@46e322a49825:/workspace/jon-examples/EmptySite# cd uart_test/
	root@46e322a49825:/workspace/jon-examples/EmptySite/uart_test# git pull origin master
*/

void fileCheck(const char *fileName);

bool file_exists (char *filename) {
  struct stat   buffer;   
  return (stat (filename, &buffer) == 0);
}

int main() {
    struct wiringXSerial_t wiringXSerial = {115200, 8, 'n', 1, 'n'};
    char buf[1024];
    int str_len = 0;
    int i;
    int cfd;

    if(wiringXSetup("duo", NULL) == -1) {
        wiringXGC();
        return -1;
    }

    if ((cfd = wiringXSerialOpen("/dev/ttyS4", wiringXSerial)) < 0) {
        printf("Open serial device failed: %d\n", cfd);
        wiringXGC();
        return -1;
    }

    wiringXSerialPuts(cfd, "Duo Serial Test.\n");

	printf("Duo UART started\n");
	
	char name[] = "foo.txt";
	
	fileCheck(name);
	
	if (file_exists(name)) {
        printf("%s exists\n", name);
	} else {
        printf("%s does not exist\n", name);
	}
	
    FILE * fp;
    int fd = open(name, O_RDWR | O_CREAT, 0777);
	if (fd != -1) {
		fp = fdopen(fd, "w");
		if (fp!=NULL) {
			fprintf(fp, "This file should be executable.\n");
			fclose(fp);
			printf("File write OK\n");
		} else {
			printf("File write open error\n");
			fp = fopen("file.txt", "w");
			if (fp != NULL)
			{
				fprintf(fp, "This file should be executable.\n");
				fclose(fp);
				printf("File write OK\n");
				
			} else {
				printf("Error opening file!\n"); 
			}
		}
	} else {
		printf("File write error\n");
	}
	
    while(1)
    {
        str_len = wiringXSerialDataAvail(cfd);
        if (str_len > 0) {
            i = 0;
			memset(buf,0, sizeof(buf));
            while (str_len--)
            {
                buf[i++] = wiringXSerialGetChar(cfd);
            }
            printf("Duo UART receive: %s\n", buf);
        }
    }

    wiringXSerialClose(cfd);

    return 0;
}

void fileCheck(const char *fileName){

    if(!access(fileName, F_OK )){
        printf("The File %s\t was Found\n",fileName);
    }else{
        printf("The File %s\t not Found\n",fileName);
    }

    if(!access(fileName, R_OK )){
        printf("The File %s\t can be read\n",fileName);
    }else{
        printf("The File %s\t cannot be read\n",fileName);
    }

    if(!access( fileName, W_OK )){
        printf("The File %s\t it can be Edited\n",fileName);
    }else{
        printf("The File %s\t it cannot be Edited\n",fileName);
    }

    if(!access( fileName, X_OK )){
        printf("The File %s\t is an Executable\n",fileName);
    }else{
        printf("The File %s\t is not an Executable\n",fileName);
    }
}