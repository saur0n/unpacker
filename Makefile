################################################################################
#   FPSX/ROFS unpacking program
################################################################################

LIBRARIES=-lstdc++ -lunix++ -lcrypto

all: fpsxdump

install: fpsxdump
	cp fpsxdump /usr/local/bin

fpsxdump: *.cpp *.hpp
	g++ -o $@ *.cpp $(LIBRARIES)

.PHONY: all install
