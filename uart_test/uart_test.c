
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <wiringx.h>

// git initgraph
// create / change files
// git add .
// git commit -m "comment"
// git push --set-upstream https://github.com/JorgenNielsen/EmptySite master

// git clone https://github.com/JorgenNielsen/EmptySite  
// root@46e322a49825:/workspace/jon-examples/EmptySite/uart_test# git pull origin master
// make
// scp uart_test root@192.168.42.1:/root/

int main() {
    struct wiringXSerial_t wiringXSerial = {115200, 8, 'n', 1, 'n'};
    char buf[1024];
    int str_len = 0;
    int i;
    int fd;

    if(wiringXSetup("duo", NULL) == -1) {
        wiringXGC();
        return -1;
    }

    if ((fd = wiringXSerialOpen("/dev/ttyS4", wiringXSerial)) < 0) {
        printf("Open serial device failed: %d\n", fd);
        wiringXGC();
        return -1;
    }

    wiringXSerialPuts(fd, "Duo Serial Test.\n");

	printf("Duo UART started\n");
	
	char name[] = "foo.txt";
    FILE * fp;
    fd = open(name, O_RDWR | O_CREAT, 0777);
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
        str_len = wiringXSerialDataAvail(fd);
        if (str_len > 0) {
            i = 0;
            while (str_len--)
            {
                buf[i++] = wiringXSerialGetChar(fd);
            }
            printf("Duo UART receive: %s\n", buf);
        }
    }

    wiringXSerialClose(fd);

    return 0;
}