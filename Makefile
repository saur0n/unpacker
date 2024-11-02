################################################################################
#   FPSX/ROFS unpacking program
################################################################################

CFLAGS=-Wall -Wno-unused
CXXFLAGS=$(CFLAGS)
LIBRARIES=-lstdc++ -lunix++ -lcrypto -lz

all: unpacker

OBJECTS=\
	build/5500.o \
	build/akuvox.o \
	build/android.o \
	build/chromium.o \
	build/fpsx.o \
	build/haier.o \
	build/images.o \
	build/main.o \
	build/REUtils.o \
	build/rofs.o \
	build/spi.o \
	build/StringUtils.o \
	build/qt.o \
	build/TypeRegistration.o

unpacker: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS) $(LIBRARIES)

build/%.o: %.cpp $(HEADERS)
	@mkdir -p `dirname $@`
	gcc -c $(CXXFLAGS) $< -o $@

clean:
	rm -rf build
	rm -f unpacker

install: unpacker
	cp unpacker /usr/local/bin

.PHONY: all clean install
