#include <iostream>
#include "REUtils.hpp"

using std::cout;
using std::endl;
using std::string;

void extract5500FileSystem(BinaryReader &is, const string &outDir, Indent indent) {
    is.skip(0x1090);
    
    for (unsigned i=0; i<10; i++) {
        is.skip(9);
        cout << indent << is.readShortUnicodeString() << endl;
    }
}
