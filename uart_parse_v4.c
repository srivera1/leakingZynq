/* 
 * sergio rivera
 * 01/19
 * 
 * 
 * At the Zynq board:
 * 	$ sudo ./pushOut 1
 * 	We are sending the string "1" with clk gating to the air!
 * 	110111000110011011
 * 	finished
 *                   
 * 
 * At local PC:
 * 	gcc -Wall -o uart uart_parse_v4.c -O3 -lm; ./uart /dev/ttyUSB0 
 * 	Time elapsed: 0.044141   9864    -9168   1 
 * 	Time elapsed: 0.046572   9760    -9064   1   
 * 	Time elapsed: 0.005032   9660    -8964   0   
 * 	Time elapsed: 0.044940   9628    -9300   1   
 * 	Time elapsed: 0.046125   9918    -9500   1   
 * 	Time elapsed: 0.045209   10223   -10500  1   
 * 	Time elapsed: 0.005014   10127   -10404  0   
 * 	Time elapsed: 0.005025   10031   -10308  0   
 * 	Time elapsed: 0.005860   9925    -10202  0   
 * 	Time elapsed: 0.044146   9827    -10104  1   
 * 	Time elapsed: 0.044144   9731    -10008  1   
 * 	Time elapsed: 0.005422   9633    -9910   0   
 * 	Time elapsed: 0.005039   9527    -9804   0   
 * 	Time elapsed: 0.044350   9927    -9408   1   
 * 	Time elapsed: 0.043920   9831    -9312   1   
 * 	Time elapsed: 0.005005   9735    -9216   0   
 * 	Time elapsed: 0.044564   9579    -9700   1   
 * 	Time elapsed: 0.043266   9435    -9556   1
 * 	                                         ^
 * 	                                         |_ binary
 *
 * /


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h> // needed for memset
#include <time.h>
#include <sys/time.h>

int main(int argc,char** argv)
{
        struct termios tio;
        struct termios stdio;
        int tty_fd;
        fd_set rdset;

        unsigned char c='D';

        printf("Please start with %s /dev/ttyS1 (for example)\n",argv[0]);
        memset(&stdio,0,sizeof(stdio));
        stdio.c_iflag=0;
        stdio.c_oflag=0;
        stdio.c_cflag=0;
        stdio.c_lflag=0;
        stdio.c_cc[VMIN]=1;
        stdio.c_cc[VTIME]=0;
        tcsetattr(STDOUT_FILENO,TCSANOW,&stdio);
        tcsetattr(STDOUT_FILENO,TCSAFLUSH,&stdio);
        fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);       // make the reads non-blocking




        memset(&tio,0,sizeof(tio));
        tio.c_iflag=0;
        tio.c_oflag=0;
        tio.c_cflag=CS8|CREAD|CLOCAL;           // 8n1, see termios.h for more information
        tio.c_lflag=0;
        tio.c_cc[VMIN]=1;
        tio.c_cc[VTIME]=5;

        tty_fd=open(argv[1], O_RDWR | O_NONBLOCK);        // O_NONBLOCK might override VMIN and VTIME, so read() may return immediately.
        cfsetospeed(&tio,B115200);            // 115200 baud
        cfsetispeed(&tio,B115200);            // 115200 baud

        tcsetattr(tty_fd,TCSANOW,&tio);
	int number=30000;
	float numberLPF1=30000;
	float numberLPF2=30000;
	int cntPlus =0;
	int cntMinus=0;
	int exponente=4;
        int numberTMP = 30000;
        int max = 0;
        int min = 0;
        int out = 0;
        int cnt[]={0,0,0,0,0,0,0,0,0,0,0,0,0};
        int i=0;
        struct timeval tval_before, tval_after, tval_result;
        gettimeofday(&tval_before, NULL);
        gettimeofday(&tval_after, NULL);
        while (c!='q') // not used yet
        {
                if (read(tty_fd,&c,1)>0) {             // if new data is available on the serial port, print it out
			if((int)c==13) {               // 
 				numberLPF1 = numberLPF1*0.99+0.01*number;             // not used here
				numberLPF2 = numberLPF2*0.999999+0.000001*numberLPF1; // not used here
				cnt[i]=number-numberTMP;   // signal derivate
				numberTMP=number;

                                if(cnt[i]>max)
                                        max=cnt[i];

                                if(cnt[i]<min)
                                        min=cnt[i];

                                printf("\r");
                                if(cnt[i]>max*0.5 & (cnt[i]>7000)) {
                                        out=1;
                                        gettimeofday(&tval_before, NULL);
                                }
                                if(cnt[i]<min*0.5 & (cnt[i]<7000)) {
                                        gettimeofday(&tval_after, NULL);
                                        out=0;
                                        timersub(&tval_after, &tval_before, &tval_result);
                                        int out = 0;
                                        if((long int)tval_result.tv_usec>9000)
                                                out = 1;
                                        if((long int)tval_result.tv_sec<1 & ((long int)tval_result.tv_usec < 150000) & ((long int)tval_result.tv_usec > 2000) )
                                                printf("Time elapsed: %ld.%06ld \t %d\t %d\t %d\t \n", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec, max, min, out);
                                }

                                        max--; min++; // continuously adapt to the adc value range 

                                i++;
                                i%=13;


                                exponente=4;
				number=0;
			} else if (((int)c > 47) & ((int)c < 60)) {
				number += (((int)c)-48)*pow(10,exponente--); // chars -> int number
				c='r';
                                usleep(10);
			}
		}
		
        }

        close(tty_fd);
}
