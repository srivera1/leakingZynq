/*
 * 
 * 
 * initial version 
 * 01/2019
 * sergio rivera
 * 
 * 
 * this program configures the clk
 * path routing (or clk gating) on
 * the PL side from a Zynq chip.
 * this is a proof of concept about 
 * how information can be leaked
 * without any extra HW from an SoC.
 *
 * the clk signal will generally no be
 * usable, (or any other digital signal)
 * since the analog bandwidth
 * of the MOSFET is not good enough.
 * but still, useful for us.
 * 
 * on the receiver side we use a
 * logarithmic detector (MAX2015)
 * as ASK detector (OOK in this
 * example) sampled by an arduino mini
 *
 * the digital HW includes an fsk
 * example too (easily adaptable to psk)
 *
 * Registers:
 * 0x41210000
 *  0x2 -> fsk random noise, 200M, WB 5MHz
 *  0x0 -> peak 200MHz
 *  0x3 -> peak 180MHz
 *  0x1 -> off 
 * 
 * 0x41200000
 * -0x1 -> peak 190MHz
 *  0x0 -> peak 180MHz
 *  0x1 -> peak 200MHz
 *  0x2 -> peak 210MHz
 *  0x3 -> peak 220MHz
 * 
 * 
 * gcc -Wall -O3 -o pushOut pushOut_v1.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>

union ufloat {
            float f;
            unsigned u;
    };

union ifloat {
    float f;
    int i;
};

int fd;
void *ptr;
unsigned page_size;
unsigned page_offset;

int readMem(void *ptr, unsigned page_offset) {
    return *((unsigned *)(ptr + page_offset));
}

int readMemInt(unsigned base_addr, unsigned offset_addr) {
    page_size=sysconf(_SC_PAGESIZE);
    unsigned hw_addr = base_addr + offset_addr;
    unsigned page_addr = (hw_addr & (~(page_size-1)));
    page_offset = hw_addr - page_addr;
    ptr = mmap(NULL, page_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, page_addr);
    return readMem(ptr,page_offset);
}


void writeMem(void *ptr, unsigned page_offset, int value) {
    *((unsigned *)(ptr + page_offset)) = value;
}

void writeMemFloat(unsigned base_addr, unsigned offset_addr, float value) {
    page_size=sysconf(_SC_PAGESIZE);
    unsigned hw_addr = base_addr + offset_addr;
    unsigned page_addr = (hw_addr & (~(page_size-1)));
    page_offset = hw_addr - page_addr;
    ptr = mmap(NULL, page_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, page_addr);
    union ufloat u3;
    u3.f=value; 
    writeMem(ptr,page_offset,u3.u);
}

void writeMemInt(unsigned base_addr, unsigned offset_addr, int value) {
    page_size=sysconf(_SC_PAGESIZE);
    unsigned hw_addr = base_addr + offset_addr;
    unsigned page_addr = (hw_addr & (~(page_size-1)));
    page_offset = hw_addr - page_addr;
    ptr = mmap(NULL, page_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, page_addr);
    writeMem(ptr,page_offset,value);
}

int main(int argc, char *argv[])
{
    /* Open /dev/mem file */
    fd = open ("/dev/mem", O_RDWR);
    if (fd < 1) {
        perror(argv[0]);
        return -1;
    }

    union charToint {
	    char c;
	    int i;
    };

    printf("We are sending the string \"%s\" with clk gating to the air!\n", argv[1]);

    union charToint a;
    int period = 50000; /* slow just for easy visualization */
    int i =0;     /* bit index */
    int j =0;     /* character index */
    
    writeMemInt(0x41210000, 0x0, 0x1);
    while(1==1) { /* while not finished */
	
	if(i%9==0){ /* if last bit of char get next char */
		j++;
		i=0;
		a.c=argv[1][j%strlen(argv[1])];
		usleep(2*period); /* wait a bit between characters */
	}
	i++;

	if(j>strlen(argv[1])) /* if last char is gone then we are done */
		break;

	int duty = period /4;
	
	if(a.i%2==0)
		duty*=3;
	a.i/=2;

	/* clk ON/OFF switching: */
        writeMemInt(0x41200000, 0x0, 0x3);
        usleep(duty);    
        writeMemInt(0x41200000, 0x0, 0x1);
        usleep(period-duty);   

    }

    printf("finished \n");
    munmap(ptr,page_size);
    return 0;
}


