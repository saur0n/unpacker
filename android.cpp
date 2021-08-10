#include <iostream>
#include "REUtils.hpp"
#include "StringUtils.hpp"

using std::cout;
using std::endl;
using std::string;

static uint32_t pages(uint32_t size, uint32_t pageSize) {
    return (size+pageSize-1)/pageSize;
}

static void extractPart(BinaryReader &is, uint32_t pageSize, uint32_t &offset, uint32_t size, const string &filename) {
    if (size) {
        BinaryReader part(is, offset*pageSize, size);
        part.extract(filename);
        offset+=pages(size, pageSize);
    }
}

void extractAndroidImage(BinaryReader &is, const string &filename) {
    string magic=is.readString(8u);
    if (magic!="ANDROID!")
        throw "invalid magic (expected 'ANDROID!')";
    
    uint32_t kernelSize=is.readIntLE();
    uint32_t kernelAddress=is.readIntLE();
    uint32_t rdSize=is.readIntLE();
    uint32_t rdAddress=is.readIntLE();
    uint32_t rd2Size=is.readIntLE();
    uint32_t rd2Address=is.readIntLE();
    uint32_t tagsAddress=is.readIntLE();
    uint32_t pageSize=is.readIntLE();
    uint32_t headerVersion=is.readIntLE();
    uint32_t osVersion=is.readIntLE();
    string productName=is.readString(16);
    BinaryReader bootCmd=is.window(512);
    uint32_t ids[8];
    for (unsigned i=0; i<8; i++)
        ids[i]=is.readIntLE();
    BinaryReader bootCmdExtra=is.window(1024);
    if (headerVersion>0)
        throw "TODO: read header fields for format v1 and v2";
    
    cout << "Format version: " << headerVersion << endl;
    for (unsigned i=0; i<8; i++)
        cout << "[" << i << "]: " << ids[i] << endl;
    
    bootCmd.extract(replaceExtension(filename, "bootcmd"));
    bootCmdExtra.extract(replaceExtension(filename, "bootcmd-extra"));
    uint32_t offset=1; // in pages, not in bytes
    extractPart(is, pageSize, offset, kernelSize, replaceExtension(filename, "kernel"));
    extractPart(is, pageSize, offset, rdSize, replaceExtension(filename, "ramdisk"));
    extractPart(is, pageSize, offset, rd2Size, replaceExtension(filename, "ramdisk2"));
}
