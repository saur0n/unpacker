################################################################################
#   FPSX/ROFS unpacking program
################################################################################

LIBRARIES=-lstdc++ -lunix++ -lcrypto -lz

all: unpacker

install: unpacker
	cp unpacker /usr/local/bin

unpacker: *.cpp *.hpp
	g++ -o $@ *.cpp $(LIBRARIES)

.PHONY: all install
