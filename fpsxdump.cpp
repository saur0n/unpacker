/*******************************************************************************
 *  FPSX/ROFS/RROM unpacking program
 *  
 *  © 2020—2021, Sauron
 ******************************************************************************/

#include <cstring>
#include <iostream>
#include "REUtils.hpp"

using namespace upp;
using std::cerr;
using std::cout;
using std::endl;
using std::string;

extern void extractROM(BinaryReader &is);
extern void extractFirmware(BinaryReader &is, const std::string &outDir, Indent indent=Indent());
extern void extractROFS(BinaryReader &is, const std::string &outDir, Indent indent=Indent());
extern void extractSPI(BinaryReader &is);

bool endsWith(std::string const &fullString, std::string const &ending) {
    if (fullString.length()>=ending.length())
        return (0==fullString.compare(fullString.length()-ending.length(), ending.length(), ending));
    else
        return false;
}

int main(int argc, char** argv) {
    try {
        if (argc == 1)
            throw "no path specified";

        std::string output;

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
        
        if (endsWith(filename, ".rom")) {
            // Akuvox firmware file
            extractROM(is);
        }
        else if (endsWith(filename, ".img")) { //HACK
            BinaryReader volume1(is, 0x12a0400, BinaryReader::ToEnd());
            extractROFS(volume1, "rofs");
            BinaryReader volume2(is, 0x3920400, BinaryReader::ToEnd());
            extractROFS(volume2, "rofs");
        }
        else if (string(filename).find(".spi")!=string::npos) { //HACK
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
