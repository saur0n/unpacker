/*******************************************************************************
 *  FPSX/ROFS/Akuvox unpacking program
 *  
 *  © 2020—2023, Sauron
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
extern void extractChromiumPackage(BinaryReader &is, const string &outDir);
extern void extractROM(BinaryReader &is);
extern void extractFirmware(BinaryReader &is, const string &outDir, Indent indent=Indent());
extern void extractROFS(BinaryReader &is, const string &outDir, Indent indent=Indent());
extern void extractSymbianImage(BinaryReader &is, const string &outDir, Indent indent=Indent());
extern void extractSPI(BinaryReader &is);
extern void extractHaierFirmware(BinaryReader &is, const string &outDir);
extern void extract5500FileSystem(BinaryReader &is, const string &outDir, Indent indent=Indent());

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
        
        if (endsWith(filename, ".5500")) {
            // Nokia 5500 file system
            extract5500FileSystem(is, output.empty()?"rofs":output);
        }
        else if (endsWith(filename, ".android")) {
            // Android sparse image
            extractAndroidImage(is, filename);
        }
        else if (endsWith(filename, ".bin")) {
            // Haier SPI Flash dump
            extractHaierFirmware(is, output.empty()?".":output);
        }
        else if (endsWith(filename, ".img")) {
            // Symbian flash image
            extractSymbianImage(is, output.empty()?"flash":output);
        }
        else if (endsWith(filename, ".pak")) {
            // Chromium resource file
            extractChromiumPackage(is, "chromium");
        }
        else if (endsWith(filename, ".rofs")) {
            // Symbian read-only file system
            extractROFS(is, output.empty()?"rofs":output);
        }
        else if (endsWith(filename, ".rom")) {
            // Akuvox firmware file
            extractROM(is);
        }
        else if (endsWith(filename, ".spi")) { //HACK
            // Symbian resource archive
            extractSPI(is);
        }
        else
            extractFirmware(is, output.empty()?"flash":output);
        
        return 0;
    }
    catch (const EOFException &ee) {
        cerr << argv[0] << ": error: premature end of file or block" << endl;
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
