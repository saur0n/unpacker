#include <iostream>
#include <unistd.h>
#include <unix++/FileSystem.hpp>
#include "REUtils.hpp"
#include "StringUtils.hpp"
#include "TypeRegistration.hpp"

using std::cout;
using std::endl;
using std::string;

static const uint32_t BB5_COMMON_HEADER_MAGIC=0x809795A3U;
static const uint32_t ROFS_MAGIC=0x53464F52U;

class FSDumpContext {
public:
    FSDumpContext(const std::string &path, uint32_t base) : path(path), base(base) {}
    FSDumpContext(const FSDumpContext &other, const std::string &name) :
        path(other.path+'/'+name), base(other.base) {}
    const std::string &getPath() const { return path; }
    uint32_t getBase() const { return base; }
    
private:
    std::string path;
    uint32_t base;
};

class Entry {
public:
    Entry(BinaryReader &is) {
        BinaryReader eis=is.window(is.readShortLE()-2);
        eis.skip(16);
        eis.skip(2);
        size=eis.readIntLE();
        address=eis.readIntLE();
        extendedAttributes=eis.readByte();
        name=eis.readShortUnicodeString();
    }
    void print(const char * type, Indent indent) {
        cout << indent << type << ": " << name << endl;
        cout << indent << ". Size: " << size << endl;
        cout << indent << ". Address: " << Hex<>(address) << endl;
    }
    uint32_t getSize() const { return size; }
    uint32_t getAddress() const { return address; }
    const std::string &getName() const { return name; }
    
private:
    uint32_t size;
    uint32_t address;
    uint8_t extendedAttributes;
    std::string name;
};

static void extractDir(BinaryReader &is, const FSDumpContext &dc, uint32_t offset, uint32_t size, Indent indent) {
    uint32_t base=dc.getBase();
    
    cout << indent << "Path=" << dc.getPath() << endl;
    mkdir(dc.getPath().c_str(), 0700);
    
    if (offset<base)
        throw "offset<base";
    offset-=base;
    BinaryReader br0(is, offset, size+2); // HACK: 2 bytes added
    size_t size2=br0.readShortLE();
    //size_t addHeaderSize=br0.readShortLE();
    //cout << indent << "AddHeaderSize: " << addHeaderSize << endl;
    if (size!=size2)
        cout << indent << "Warning: " << size << "<>" << size2 << endl;
    BinaryReader br=br0.window(size2);
    br.readByte(); // padding;
    uint8_t firstEntryOffset=br.readByte();
    uint32_t fileBlockAddress=br.readIntLE();
    uint32_t fileBlockSize=br.readIntLE();
    
    if (firstEntryOffset!=12) cout << indent << "WARNING!" << endl;
    cout << indent << "First entry offset: " << unsigned(firstEntryOffset) << endl;
    cout << indent << "File block address: " << Hex<>(fileBlockAddress) << endl;
    cout << indent << "File block size: " << fileBlockSize << endl;
    
    while (!br.atEnd(2)) {
        Entry entry(br);
        entry.print("SubDir", indent);
        FSDumpContext dc0(dc, entry.getName());
        extractDir(is, dc0, entry.getAddress(), entry.getSize(), indent);
    }
    
    // Files
    if (fileBlockAddress) {
        if (fileBlockAddress<base)
            throw "fileBlockAddress<base";
        fileBlockAddress-=base;
        BinaryReader fbis(is, fileBlockAddress, fileBlockSize+2);
        while (!fbis.atEnd(2)) {
            Entry entry(fbis);
            entry.print("File", indent);
            uint32_t realAddress=entry.getAddress();
            if (realAddress<base)
                cout << indent << "    [SKIP]" << endl;
            else {
                realAddress-=base;
                BinaryReader fileReader(is, realAddress, entry.getSize());
                fileReader.extract(dc.getPath()+'/'+entry.getName(), true);
            }
        }
    }
}

void extractROFS(BinaryReader &is, const string &outDir, Indent indent) {
    uint32_t magic=is.readIntLE();
    cout << "magic " << Hex<>(magic) << endl;
    if (magic==BB5_COMMON_HEADER_MAGIC) {
        cout << indent << "This is a BB5 image" << endl;
        BinaryReader rofs(is, 1024, BinaryReader::END);
        extractROFS(rofs, outDir, indent);
    }
    else if (magic!=ROFS_MAGIC)
        throw "wrong ROFS magic number";
    
    uint8_t headerSize=is.readByte();
    uint8_t reversed=is.readByte();
    BinaryReader wis=is.window(headerSize-6);
    uint16_t formatVersion=wis.readShort();
    uint32_t dirTreeOffset=wis.readIntLE();    // offset to start of directory structure
    uint32_t dirTreeSize=wis.readIntLE();        // size in bytes of directory
    uint32_t dirFileEntriesOffset=wis.readIntLE();    // offset to start of file entries
    uint32_t dirFileEntriesSize=wis.readIntLE();    // size in bytes of file entry block
    int64_t time=wis.readLongLE();
    uint8_t versionMajor=wis.readByte();
    uint8_t versionMinor=wis.readByte();
    uint16_t versionBuild=wis.readShort();
    uint32_t image_size=wis.readInt(); // rofs image size
    uint32_t checksum=wis.readInt();
    uint32_t max_image_size=wis.readInt();
    
    cout << "Header size: " << unsigned(headerSize) << endl;
    cout << "Format version: " << formatVersion << endl;
    cout << "Image version: " << unsigned(versionMajor) << '.' << unsigned(versionMinor) << '.' << versionBuild << endl;
    cout << "Tree: " << Hex<>(dirTreeOffset) << '/' << dirTreeSize << endl;
    cout << "File entries: " << Hex<>(dirFileEntriesOffset) << '/' << dirFileEntriesOffset << endl;
    //cout << "VOffset: " << Hex<>(dirTreeOffset-0x30) << endl;
    
    FSDumpContext dc(outDir, dirTreeOffset-0x30);
    extractDir(is, dc, dirTreeOffset, dirTreeSize, indent);
}

void extractVolumes(BinaryReader &is, const string &outDir, Indent indent) {
    unsigned i=0;
    for (bool end=false; !end; i++) {
        uint32_t offset=is.readIntLE();
        uint32_t size=is.readIntLE();
        uint32_t unknown1=is.readIntLE();
        uint32_t unknown2=is.readIntLE();
        uint32_t unknown3=is.readIntLE();
        string name=trim(is.readString(12));
        
        if ((offset==0xFFFFFFFF)&&(size==0xFFFFFFFF))
            end=true;
        else {
            cout << indent << "Volume " << i << "\n";
            Indent shift(indent);
            cout << shift << "Data: " << offset << ':' << size << endl;
            if (unknown1)
                cout << shift << "Unknown1: " << unknown1 << endl;
            if (unknown2)
                cout << shift << "Unknown2: " << unknown2 << endl;
            if (unknown3)
                cout << shift << "Unknown3: " << unknown3 << endl;
            cout << shift << "Name: " << name << endl;
            
            // Extract the partition
            if (offset!=0xFFFFFFFF) {
                if (is.getSize()<offset+size) {
                    cout << shift << "Truncating the partition" << endl;
                    size=is.getSize()-offset;
                }
                BinaryReader pis(is, offset, size);
                pis.extract(outDir+"/"+name+".img");
                
                if (name=="SOS-TOC") {
                    BinaryReader nis(is, offset, BinaryReader::END);
                    extractVolumes(nis, outDir, indent);
                }
            }
        }
    }
}

void extractSymbianImage(BinaryReader &is, const string &outDir, Indent indent) {
    // Locate partition table
    size_t partitionTableOffset=0;
    BinaryReader isl(is);
    bool empty=true;
    while (empty) {
        string entry=isl.readString(32);
        for (auto i=entry.begin(); i!=entry.end(); ++i)
            if (*i)
                empty=false;
        if (empty)
            partitionTableOffset+=32;
    }
    
    cout << "Partition table found at " << partitionTableOffset << endl;
    BinaryReader contents(is, partitionTableOffset, BinaryReader::END);
    extractVolumes(contents, outDir, indent);
}

static bool detect(BinaryReader &is, const string &filename) {
    if (endsWith(filename, ".rofs"))
        return true;
    else if (is.readIntLE()==BB5_COMMON_HEADER_MAGIC)
        return true;
    else
        return false;
}

static void extract(BinaryReader &is, const string &outDir) {
    return extractROFS(is, outDir, Indent());
}

TR(rofs);

