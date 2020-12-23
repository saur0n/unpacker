################################################################################
#   FPSX/ROFS unpacking program
################################################################################

all: fpsxdump

install: fpsxdump
	cp fpsxdump /usr/local/bin

fpsxdump: *.cpp *.hpp
	g++ -o $@ *.cpp -lstdc++ -lunix++

.PHONY: all install
