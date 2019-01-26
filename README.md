# leakingZynq
This is a proof of concept about how information can be leaked without any extra HW from an SoC

## FPGA configuration:
<br />$ echo 0 > /sys/class/fpga_manager/fpga0/flags
<br />$ cp leakingZynq_demo_v0.bin /lib/firmware
<br />$ cd /lib/firmware
<br />$ sudo echo leakingZynq_demo_v0.bin > /sys/class/fpga_manager/fpga0/firmware

## code compilation and execution
<br />$ cd ~/leakingZynq
<br />$ gcc -Wall -O3 -o pushOut pushOut_v1.c
<br />$ sudo ./pushOut myString
<br />
<br /> for more info check the file pushOut_v1.c 
