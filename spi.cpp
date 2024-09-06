#include <iostream>
#include "REUtils.hpp"

using std::cerr;
using std::cout;
using std::endl;
using std::string;

static const uint32_t SPI_MAGIC=0x10205C2B;

void extractSPI(BinaryReader &is) {
    // SPI header
    uint32_t magic=is.readIntLE();
    if (magic!=SPI_MAGIC)
        throw "wrong SPI file magic";
    
    cout << "Type: " << Hex<>(is.readIntLE()) << endl;
    is.skip(24);
    
    while (!is.atEnd()) {
        is.align(4);
        uint32_t fileNameLength=is.readIntLE();
        uint32_t fileSize=is.readIntLE();
        string filename=is.readString(fileNameLength);
        BinaryReader entry=is.window(fileSize);
        
        cout << "Entry: " << filename << endl;
        uint32_t uid0=entry.readIntLE();
        cout << "    UID0: " << Hex<>(uid0) << endl;
        uint32_t uid1=entry.readIntLE();
        cout << "    UID0: " << Hex<>(uid1) << endl;
    }
}
