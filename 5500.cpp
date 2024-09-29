#include <iostream>
#include "REUtils.hpp"
#include "StringUtils.hpp"
#include "TypeRegistration.hpp"

using std::cout;
using std::endl;
using std::string;

static bool detect(BinaryReader &is, const string &filename) {
    return endsWith(filename, ".5500");
}

static void extract(BinaryReader &is, const string &outDir) {
    Indent indent;
    is.skip(0x1090);
    
    for (unsigned i=0; i<10; i++) {
        is.skip(9);
        cout << indent << is.readShortUnicodeString() << endl;
    }
}

TR(5500);
