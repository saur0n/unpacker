/*******************************************************************************
 *  Unpacker for Qt installer framework
 *  
 *  © 2024, Sauron <fpsxdump@saur0n.science>
 ******************************************************************************/

#include <iostream>
#include <unix++/File.hpp>
#include <unix++/FileSystem.hpp>
#include "REUtils.hpp"
#include "StringUtils.hpp"
#include "TypeRegistration.hpp"

using std::cout;
using std::string;
using std::wstring;

static const uint32_t MAGIC=0x71726573;

static wstring readName(BinaryReader &names, uint32_t offset) {
    BinaryReader is(names, offset, BinaryReader::END);
    uint16_t length=is.readShort();
    uint64_t hash=is.readInt();
    return is.readWideString(length);
}

static bool detect(BinaryReader &is, const string &filename) {
    return endsWith(filename, ".rcc");
}

static void extract(const string &outDir, const string &prefix,
        BinaryReader &tree, BinaryReader &data, BinaryReader &names,
        unsigned index=0) {
    BinaryReader node(tree, 14*index, BinaryReader::END);
    uint32_t nameOff=node.readInt();
    uint16_t flags=node.readShort();
    
    wstring name=readName(names, nameOff);
    string path=prefix;
    if (index) {
        path+='/';
        path+=convert(name);
    }
    string outPath=outDir+'/'+path;
    
    if (flags&2) {
        // directory
        cout << "Directory: " << convert(name) << "\n";
        uint32_t nChildren=node.readInt();
        uint32_t childOffset=node.readInt();
        
        try {
            upp::FileSystem::mkdir(outPath.c_str(), 0700);
        }
        catch (...) {}
        
        for (uint32_t i=0; i<nChildren; i++)
            extract(outDir, path, tree, data, names, childOffset+i);
    }
    else {
        // regular file
        cout << "File: " << convert(name) << "\n";
        uint16_t country=node.readShort();
        uint16_t language=node.readShort();
        uint32_t offset=node.readInt();
        
        BinaryReader item(data, offset, BinaryReader::END);
        uint32_t length=item.readInt();
        ByteArray resource;
        
        if (flags&1) {
            uint32_t uncompressedLength=item.readInt();
            resource=item.read(length-sizeof(uint32_t));
            resource=uncompress(resource, uncompressedLength);
        }
        else
            resource=item.read(length);
        
        cout << "Path: " << outPath << "\n";
        upp::File file(outPath.c_str(), O_WRONLY|O_CREAT|O_TRUNC);
        file.write(resource.data(), resource.size());
    }
}

static void extract(BinaryReader &is, const string &outDir) {
    BinaryReader header(is);
    if (header.readInt()!=MAGIC)
        throw "bad magic";
    
    if (1!=header.readInt())
        throw "unknown rcc version";
    
    uint32_t tOff=header.readInt();
    BinaryReader tree(is, tOff, BinaryReader::END);
    uint32_t dOff=header.readInt();
    BinaryReader data(is, dOff, BinaryReader::END);
    uint32_t nOff=header.readInt();
    BinaryReader names(is, nOff, BinaryReader::END);
    
    extract(outDir.empty()?".":outDir, string(), tree, data, names);
    cout << "DONE\n";
}

TR(qt);
