CC := $(CC)
CXX := $(CXX)
AR := $(AR)
LD := $(LD)
CPPFLAGS := $(CPPFLAGS)
CFLAGS := $(CFLAGS)
CXXFLAGS := $(CXXFLAGS)
LDFLAGS := $(LDFLAGS)

.PHONY: all clean

all: writer

writer: writer.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^

clean:
	rm -rf writer *.o

