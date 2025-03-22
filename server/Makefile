CC := aarch64-none-linux-gnu-gcc
CXX := $(CXX)
AR := $(AR)
LD := $(LD)
CPPFLAGS := $(CPPFLAGS)
CFLAGS := $(CFLAGS) 
CXXFLAGS := $(CXXFLAGS)
LDFLAGS := $(LDFLAGS)

.PHONY=all clean
all: aesdsocket
aesdsocket: aesdsocket.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -Wall -g -o $@ $^

clean:
	rm aesdsocket
