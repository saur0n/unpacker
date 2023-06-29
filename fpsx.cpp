#include <iostream>
#include <unix++/FileSystem.hpp>
#include "REUtils.hpp"

using std::cout;
using std::endl;
using std::string;
using upp::File;

enum BlockType : uint8_t {
    BLOCK_TYPE_BINARY=0x17,
    BLOCK_TYPE_ROFS_HASH=0x27,
    BLOCK_TYPE_CORE_CERT=0x28,
    BLOCK_TYPE_H2E=0x2E,
    BLOCK_TYPE_H30=0x30,
    BLOCK_TYPE_H3A=0x3A,
    BLOCK_TYPE_H49=0x49
};

struct Property {
    uint8_t key;
    const char * name;
    bool large;
    uint8_t presentation;
};

static const Property PROPERTIES[]={
    {13,  "MORE_ROOT_KEY_HASH_MORE",    false,  0},
    {18,  "ERASE_AREA_BB5",             false,  0},
    {19,  "ONENAND_SUBTYPE_UNK2",       false,  0},
    {25,  "FORMAT_PARITION_BB5",        false,  0},
    {47,  "PARTITION_INFO_BB5",         false,  0},
    {194, "CMT_TYPE",                   false,  1},
    {195, "CMT_ALGO",                   false,  1},
    {200, "ERASE_DCT5",                 false,  0},
    {201, "UNKC9",                      false,  0},
    {205, "SECONDARY_SENDING_SPEED",    false,  0},
    {206, "ALGO_SENDING_SPEED",         false,  0},
    {207, "PROGRAM_SENDING_SPEED",      false,  0},
    {209, "MESSAGE_SENDING_SPEED",      false,  0},
    {212, "CMT_SUPPORTED_HW",           false,  0},
    {225, "APE_SUPPORTED_HW",           false,  0},
    {228, "UNKE4_IMPL",                 true,   0},
    {229, "UNKE5",                      true,   0},
    {230, "DATE_TIME",                  false,  0},
    {231, "APE_PHONE_TYPE",             false,  0},
    {232, "APE_ALGORITHM",              false,  1},
    {234, "UNKEA",                      true,   0},
    {236, "UNKEC",                      false,  0},
    {237, "UNKED",                      false,  0},
    {238, "ARRAY",                      true,   3},
    {243, "UNKF3_IMPL",                 true,   2},
    {244, "DESCR",                      false,  1},
    {246, "UNKF6",                      false,  0},
    {247, "UNKF7",                      false,  0},
    {250, "UNKFA_IMPL",                 false,  0}
};

void extractFirmware(BinaryReader &is, const std::string &path, Indent indent=Indent());

static const Property &getProperty(uint8_t key) {
    static const Property DEFAULT={0, nullptr, false, 0};
    for (size_t i=0; i<sizeof(PROPERTIES)/sizeof(PROPERTIES[0]); i++)
        if (PROPERTIES[i].key==key)
            return PROPERTIES[i];
    return DEFAULT;
}

static void dumpTLV(BinaryReader &is, const string &path, Indent indent) {
    uint32_t nProperties=is.readInt();
    for (uint32_t i=0; i<nProperties; i++) {
        uint8_t key=is.readByte();
        const Property &property=getProperty(key);
        
        size_t length=property.large?is.readShort():is.readByte();
        string value=is.readString(length);
        
        cout << indent << "Property " << Hex<>(key);
        if (property.name)
            cout << ' ' << property.name;
        cout << ": ";
        
        if (value.empty()) {
            if (key==0xfa) {
                cout << endl;
                BinaryReader iis=is.window(is.readInt());
                //iis.extract(path+"/toolbox.0");
                extractFirmware(iis, path+"/toolbox", indent);
                continue;
            }
            else
                cout << is.readInt();
        }
        else if (property.presentation==1) {
            cout << value;
        }
        else if (property.presentation==2) {
            string filename=path+"/property"+std::to_string(key);
            File fout(filename.c_str(), O_WRONLY|O_TRUNC|O_CREAT);
            fout.write(&value[0], value.size());
            cout << "saved to " << filename;
        }
        else if (property.presentation==3) {
            // TODO: nested TLV block
            cout << toHexString(value);
        }
        else
            cout << toHexString(value);
        
        cout << endl;
    }
}

static void dumpBlock(BinaryReader &is, const string &path, Indent indent) {
    uint8_t ctype=is.readByte();
    uint8_t unknown0=is.readByte();
    uint8_t btype=is.readByte();
    uint8_t headerSize=is.readByte();
    
    cout << indent << "!" << Hex<>(is.debug()) << endl;
    cout << indent << "CType: " << Hex<>(ctype) << endl;
    cout << indent << "Padding: " << Hex<>(unknown0) << endl;
    cout << indent << "Type: " << Hex<>(btype) << endl;
    cout << indent << "HeaderSize: " << unsigned(headerSize) << endl;
    
    BinaryReader wis=is.window(headerSize);
    is.readByte();
    
    if (btype==BLOCK_TYPE_BINARY) {
        uint8_t memType=wis.readByte();
        uint16_t unknown1=wis.readShort();
        uint32_t checksum=wis.readShort();
        uint8_t padding=wis.readByte();
        uint32_t length=wis.readInt();
        uint32_t offset=wis.readInt();
        uint8_t unknown2=wis.readByte();
        
        cout << indent << "MemType: " << Hex<>(memType) << endl;
        cout << indent << "Unknown1: " << unknown1 << endl;
        cout << indent << "Checksum: " << checksum << endl;
        cout << indent << "Data block: " << offset << ":" << length << endl;
        cout << indent << "Unknown2: " << unsigned(unknown2) << endl;
        
        is.extract(path+"/rofs.img", offset, length);
    }
    else if (btype==BLOCK_TYPE_ROFS_HASH) {
        // Toolbox?
        string sha1=wis.readString(20);
        string description=wis.readString(12);
        uint8_t memType=wis.readByte();
        uint16_t unknown=wis.readShort();
        uint16_t checksum=wis.readShort();
        uint32_t length=wis.readInt();
        uint32_t offset=wis.readInt();
        uint8_t unknown2=wis.readByte();
        
        cout << indent << "Description: " << description << endl;
        cout << indent << "MemType: " << Hex<>(memType) << endl;
        cout << indent << "Unknown: " << unknown << endl;
        cout << indent << "Checksum: " << checksum << endl;
        cout << indent << "Data block: " << offset << ":" << length << endl;
        cout << indent << "Unknown2: " << unsigned(unknown2) << endl;
        
        is.extract(path+"/rofs.img", offset, length);
    }
    else if (btype==BLOCK_TYPE_CORE_CERT) {
        // Certificate
        string sha1=wis.readString(20);
        string description=wis.readString(12);
        uint8_t memType=wis.readByte();
        uint16_t unknown=wis.readShort();
        uint16_t checksum=wis.readShort();
        uint32_t length=wis.readInt();
        uint32_t offset=wis.readInt();
        string sha12=wis.readString(20);
        uint16_t unknown2=wis.readShort();
        uint8_t unknown3=wis.readByte();
        
        cout << indent << "Description: " << description << endl;
        cout << indent << "MemType: " << Hex<>(memType) << endl;
        cout << indent << "Unknown: " << unknown << endl;
        cout << indent << "Checksum: " << checksum << endl;
        cout << indent << "Data block: " << offset << ":" << length << endl;
        cout << indent << "Unknown2: " << unknown2 << endl;
        cout << indent << "Unknown3: " << unsigned(unknown3) << endl;
        
        if ((int)offset==-1)
            is.extract(path+"/"+description+".img", 0, length);
        else
            is.extract(path+"/rofs.img", offset, length);
    }
    else if (btype==BLOCK_TYPE_H2E) {
        uint8_t memType=wis.readByte();
        for (unsigned i=0; i<4; i++)
            cout << indent << "Unknown[" << i << "]: " << Hex<>(wis.readByte()) << endl;
        string description=wis.readString(12);
        uint32_t length=wis.readInt();
        uint32_t offset=wis.readInt();
        uint8_t unknown=wis.readByte();
        
        cout << indent << "MemType: " << Hex<>(memType) << endl;
        cout << indent << "Description: " << description << endl;
        cout << indent << "Length: " << length << endl;
        cout << indent << "Offset: " << offset << endl;
        cout << indent << "Unknown: " << unsigned(unknown) << endl;
        
        is.extract(path+'/'+"userarea.img", offset, length);
    }
    else if (btype==BLOCK_TYPE_H30) {
        throw "BLOCK_TYPE_H30 is not supported yet";
    }
    else if (btype==BLOCK_TYPE_H3A) {
        uint8_t memType=wis.readByte();
        uint16_t unknown=wis.readShort();
        uint16_t checksum=wis.readShort();
        string description=wis.readString(12);
        
        cout << indent << "MemType: " << Hex<>(memType) << endl;
        cout << indent << "Unknown: " << unsigned(unknown) << endl;
        cout << indent << "Checksum: " << checksum << endl;
        cout << indent << "Description: " << description << endl;
    }
    else if (btype==BLOCK_TYPE_H49) {
        throw "BLOCK_TYPE_H49 is not supported yet";
    }
    else {
        throw "unknown block type";
    }
    
    if (!wis.atEnd())
        cout << indent << "Block header was not fully read" << endl;
}

void extractFirmware(BinaryReader &is, const string &path, Indent indent) {
    uint8_t signature=is.readByte();
    uint32_t headerSize=is.readInt();
    
    mkdir(path.c_str(), 0700);
    
    cout << indent << "Magic: " << unsigned(signature) << endl;
    cout << indent << "Header size: " << headerSize << endl;
    
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
