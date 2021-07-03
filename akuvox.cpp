#include <iostream>
#include "REUtils.hpp"

using std::cout;
using std::endl;
using std::string;

void extractROM(BinaryReader &is) {
    uint32_t sections=1, section=0;
    while (section++<sections) {
        string name=is.readString(4);
        uint32_t length=is.readIntLE();
        uint32_t crc=is.readIntLE();
        BinaryReader window=is.window(length);
        cout << name << " (" << length << " bytes)" << endl;
        
        if (name=="MORR") {
            uint32_t unknown01=window.readIntLE();
            uint32_t unknown02=window.readIntLE();
            sections+=window.readIntLE();
            cout << "Number of segments: " << sections << endl;
        }
        else if (name=="TAPR") {
            uint32_t a=window.readIntLE();
            uint32_t b=window.readIntLE();
            uint32_t c=window.readIntLE();
            uint32_t nr=window.readIntLE();
            uint32_t e=window.readIntLE();
            uint32_t dataLength=window.readIntLE();
            uint32_t g=window.readIntLE();
            uint32_t offset=window.readIntLE();
            if (a)
                cout << "a=" << a << ", ";
            if (b)
                cout << "b=" << b << ", ";
            cout << "#" << nr << ": off=" << offset << "; len=" << dataLength << endl;
            
            BinaryReader data(is, offset, dataLength);
            data.extract("seg"+std::to_string(section));
        }
        else
            cout << "Unknown section `" << name << "`" << endl;
    }
}
