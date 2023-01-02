/*******************************************************************************
 *  FPSX/ROFS/Akuvox unpacking program
 *  
 *  © 2020—2021, Sauron
 ******************************************************************************/

#include <cstring>
#include <iostream>
#include "REUtils.hpp"
#include "StringUtils.hpp"

using namespace upp;
using std::cerr;
using std::cout;
using std::endl;
using std::string;

extern void extractAndroidImage(BinaryReader &is, const string &filename);
extern void extractROM(BinaryReader &is);
extern void extractFirmware(BinaryReader &is, const string &outDir, Indent indent=Indent());
extern void extractROFS(BinaryReader &is, const string &outDir, Indent indent=Indent());
extern void extractSPI(BinaryReader &is);
extern void extractHaierFirmware(BinaryReader &is);

int main(int argc, char** argv) {
    try {
        if (argc == 1)
            throw "no path specified";

        string output;

        for (uint32_t i = 1; i < argc - 1; i++) {
            if (strncmp(argv[i], "-o", 2) == 0) {
                i++;
                output = argv[i];
            }
        }
        
        const char * filename = argv[argc-1];
        File file(filename);
        BinaryReader is(file);
        uint32_t magic=BinaryReader(is).readInt();
        
        if (endsWith(filename, ".android")) {
            // Android sparse image
            extractAndroidImage(is, filename);
        }
        else if (endsWith(filename, ".bin")) {
            // Haier SPI Flash dump
            extractHaierFirmware(is);
        }
        else if (endsWith(filename, ".rom")) {
            // Akuvox firmware file
            extractROM(is);
        }
        else if (endsWith(filename, ".img")) {
            BinaryReader volume1(is, 0x12a0400, BinaryReader::ToEnd());
            extractROFS(volume1, "rofs");
            BinaryReader volume2(is, 0x3920400, BinaryReader::ToEnd());
            extractROFS(volume2, "rofs");
        }
        else if (endsWith(filename, ".spi")) { //HACK
            extractSPI(is);
        }
        else
            extractFirmware(is, "flash");
        
        return 0;
    }
    catch (const EOFException &ee) {
        cerr << argv[0] << ": error: premature and of file or block" << endl;
        return 1;
    }
    catch (const char * error) {
        cerr << argv[0] << ": error: " << error << endl;
        return 1;
    }
    catch (const std::string &error) {
        cerr << argv[0] << ": error: " << error << endl;
        return 1;
    }
    catch (const std::exception &e) {
        cerr << argv[0] << ": error: " << e.what() << endl;
        return 1;
    }
}
