/*******************************************************************************
 *  FPSX/ROFS/Akuvox unpacking program
 *  
 *  © 2020—2024, Sauron
 ******************************************************************************/

#include <cstring>
#include <iostream>
#include "REUtils.hpp"
#include "StringUtils.hpp"
#include "TypeRegistration.hpp"

using namespace upp;
using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::vector;

extern void extractAndroidImage(BinaryReader &is, const string &filename);
extern void extractChromiumPackage(BinaryReader &is, const string &outDir);
extern void extractFirmware(BinaryReader &is, const string &outDir, Indent indent=Indent());
extern void extractSymbianImage(BinaryReader &is, const string &outDir, Indent indent=Indent());
extern void extractHaierFirmware(BinaryReader &is, const string &outDir);
extern void extract5500FileSystem(BinaryReader &is, const string &outDir, Indent indent=Indent());

int main(int argc, char** argv) {
    try {
        if (argc == 1)
            throw "no path specified";
        
        string output;
        string type;
        vector<const char *> files;
        
        for (int i=1; i<argc; i++) {
            const char * arg=argv[i];
            
            if (strncmp(arg, "-l", 2) == 0) {
                cerr << "Supported file types: ";
                auto list=TypeRegistration::list();
                for (auto i=list.begin(); i!=list.end(); ++i)
                    cerr << "\e[32m" << *i << "\e[0m ";
                cerr << endl;
            }
            else if (strncmp(arg, "-o", 2) == 0) {
                output = argv[++i];
            }
            else if (strncmp(arg, "-t", 2) == 0) {
                type = argv[++i];
            }
            else
                files.emplace_back(arg);
        }
        
        for (auto i=files.begin(); i!=files.end(); ++i) {
            const char * filename=*i;
            File file(filename);
            BinaryReader is(file);
            
            if (type.empty()) {
                if (endsWith(filename, ".android")) {
                    // Android sparse image
                    extractAndroidImage(is, filename);
                }
                else if (endsWith(filename, ".img")) {
                    // Symbian flash image
                    extractSymbianImage(is, output.empty()?"flash":output);
                }
                else {
                    auto extract=TypeRegistration::resolve(is, filename);
                    if (extract)
                        extract(is, output);
                    else
                        cerr << filename << ": cannot determine file type" << endl;
                }
            }
            else {
                // Backend name was explicitly specified via the command line
                auto extract=TypeRegistration::get(type);
                if (extract)
                    extract(is, output);
                else {
                    cerr << "Backend `" << type << "` does not exist. Try `" <<
                        argv[0] << " -l` to list all backends." << endl;
                }
            }
        }
        
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
