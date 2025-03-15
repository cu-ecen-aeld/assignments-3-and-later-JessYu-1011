CPL := gcc
.PHONY: all clean
ifeq ($(CROSS_COMPILE), aarch64-none-linux-gnu-)
	CPL := aarch64-none-linux-gnu-gcc
else 
	CPL := gcc
endif

all: writer.c
	$(CPL) -o writer writer.c
clean:
	rm -rf writer *.o

