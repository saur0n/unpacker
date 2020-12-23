#include <iostream>
#include <unix++/FileSystem.hpp>
#include "REUtils.hpp"

using std::cout;
using std::endl;
using std::string;

void extractFirmware(BinaryReader &is, const std::string &path, Indent indent=Indent());

static string trim(const string &str) {
    size_t length=str.length();
    while ((length>0)&&(str[length-1]=='\0'))
        length--;
    return str.substr(0, length);
}

static void dumpTLV(BinaryReader &is, const string &path, Indent indent) {
    uint32_t nProperties=is.readInt();
    for (uint32_t i=0; i<nProperties; i++) {
        uint8_t key=is.readByte();
        string value=is.readShortString();
        cout << indent << "Property " << Hex<>(key) << ": ";
        if (value.empty()) {
            if (key==0xee) {
                is.skip(3);
                uint8_t len=is.readByte();
                is.skip(len);
            }
            else if (key==0xfa) {
                cout << endl;
                BinaryReader iis=is.window(is.readInt());
                extractFirmware(iis, path+"/toolbox", indent);
                continue;
            }
            else
                cout << is.readInt();
        }
        else if (key==0xc2||key==0xc3||key==0xe7||key==0xe8||key==0xf4)
            cout << value;
        else
            cout << "(binary, " << value.length() << " bytes)";
        cout << endl;
    }
}

static void dumpBlock(BinaryReader &is, const string &path, Indent indent) {
    uint8_t ctype=is.readByte();
    uint8_t unknown0=is.readByte();
    uint8_t btype=is.readByte();
    uint8_t headerSize=is.readByte();
    uint8_t memType=is.readByte();
    
    cout << "!" << Hex<>(is.debug()) << endl;
    cout << indent << "CType: " << Hex<>(ctype) << endl;
    cout << indent << "Unknown0: " << Hex<>(unknown0) << endl;
    cout << indent << "Type: " << Hex<>(btype) << endl;
    cout << indent << "HeaderSize: " << unsigned(headerSize) << endl;
    cout << indent << "MemType: " << Hex<>(memType) << endl;
    
    BinaryReader wis=is.window(headerSize);
    if (btype==0x17) {
        uint8_t unknown1=wis.readByte();
        uint32_t unknown2=wis.readInt();
        uint32_t size=wis.readInt();
        uint32_t offset=wis.readInt();
        uint16_t unknown3=wis.readByte();
        cout << indent << "Unknown: " << unsigned(unknown1) << "/" << unknown2 << "/" << unsigned(unknown3) << endl;
        is.extract(path+'/'+"rofs.img", offset, size);
    }
    else if (btype==0x27) {
        // Toolbox?
        wis.skip(19);
        string description=wis.readString(12);
        wis.skip(5);
        uint32_t length=wis.readInt();
        wis.skip(5);
        is.extract(path+'/'+trim(description)+".img", 0, length);
        
        cout << indent << "Description: " << description << endl;
        cout << indent << "Length: " << length << endl;
    }
    else if (btype==0x28) {
        // Certificate
        wis.skip(36);
        uint32_t length=wis.readInt();
        is.skip(length);
    }
    else if (btype==0x2e) {
        for (unsigned i=0; i<4; i++)
            cout << indent << "Unknown[" << i << "]: " << Hex<>(wis.readByte()) << endl;
        string description=wis.readString(12);
        uint32_t length=wis.readInt();
        uint32_t offset=wis.readInt();
        uint8_t unknown=wis.readByte();
        is.extract(path+'/'+"userarea.img", offset, length);
        cout << indent << "Description: " << description << endl;
        cout << indent << "Length: " << length << endl;
        cout << indent << "Offset: " << offset << endl;
        cout << indent << "Unknown: " << unsigned(unknown) << endl;
    }
    else if (btype==0x3a) {
        // unknown
        cout << indent << "Skipping block 0x3A" << endl;
    }
    else {
        throw "unknown block type";
    }
}

void extractFirmware(BinaryReader &is, const string &path, Indent indent) {
    uint8_t signature=is.readByte();
    uint32_t headerSize=is.readInt();
    
    mkdir(path.c_str(), 0700);
    
    // Header
    BinaryReader wis=is.window(headerSize);
    dumpTLV(wis, path, indent);
    if (!wis.atEnd())
        cout << "Header is not fully read (@" << Hex<unsigned>(wis.tell()) << ")" << endl;
    
    // Blocks
    for (unsigned i=0; !is.atEnd(); i++) {
        cout << indent << "Block #" << i << ": " << endl;
        dumpBlock(is, path, indent);
    }
}
