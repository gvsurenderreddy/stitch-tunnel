ifeq ($(STITCH_TUN_TARGET), RASPBERRYPI)
CC = arm-linux-gnueabihf-gcc
CFLAGS = --sysroot=/home/asridharan/raspberrypi/rootfs -Wall -g -D$(OS) -DTARGET_RASPBERRYPI
else
CC = gcc
CFLAGS=-Wall -g -D$(OS) -DTARGET_X86_64
OPENSSL_SUPPORT_LIB=-lssl -lcrypto
OPENSSL_SUPPORT=1
endif
