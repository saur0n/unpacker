################################################################################
#   FPSX/ROFS unpacking program
################################################################################

CFLAGS=-Wall -Wno-unused
CXXFLAGS=$(CFLAGS)
LIBRARIES=-lstdc++ -lunix++ -lcrypto -lz

all: unpacker

install: unpacker
	cp unpacker /usr/local/bin

unpacker: *.cpp *.hpp
	g++ $(CXXFLAGS) -o $@ *.cpp $(LIBRARIES)

clean:
	rm -f unpacker

.PHONY: all clean install
