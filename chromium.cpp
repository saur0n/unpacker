/*******************************************************************************
 *  Chromium resource package unpacker
 *  
 *  © 2023—2024, Sauron <unpacker@saur0n.science>
 ******************************************************************************/

#include <iostream>
#include <sys/stat.h>
#include <vector>
#include "REUtils.hpp"
#include "StringUtils.hpp"
#include "TypeRegistration.hpp"

using std::cout;
using std::endl;
using std::string;
using std::vector;

static const uint16_t GZIP_MAGIC=0x8B1F;
static const uint16_t BROTLI_MAGIC=0x9B1E;

struct Resource {
    uint16_t resourceId;
    uint32_t fileOffset;
};

static void extract(const string &filename, const ByteArray &ba) {
    upp::File file(filename.c_str(), O_CREAT|O_WRONLY|O_TRUNC);
    file.write(ba.data(),ba.size());
}

static bool detect(BinaryReader &is, const string &filename) {
    return endsWith(filename, ".pak");
}

static void extract(BinaryReader &is, const string &outDir) {
    uint32_t version=is.readIntLE();
    uint8_t encoding=0;
    uint16_t nResources=0, nAliases=0;
    
    // Read the header
    if (version==4) {
        nResources=is.readIntLE();
        encoding=is.readByte();
    }
    else if (version==5) {
        encoding=is.readByte();
        is.skip(3);
        nResources=is.readShortLE();
        nAliases=is.readShortLE();
    }
    else
        throw "unknown file format version";
    
    // Read the resource table
    vector<Resource> resources(nResources);
    for (uint16_t i=0; i<nResources; i++) {
        uint16_t resourceId=is.readShortLE();
        uint32_t fileOffset=is.readIntLE();
        
        cout << "Entry #" << i << ": resource " << resourceId << endl;
        resources[i].resourceId=resourceId;
        resources[i].fileOffset=fileOffset;
    }
    
    // Read the alias table
    for (uint16_t i=0; i<nAliases; i++) {
        uint16_t resourceId=is.readShortLE();
        uint16_t entryIndex=is.readShortLE();
        
        cout << "Alias #" << i << ": resource " << resourceId << endl;
    }
    
    // Finally, unpack the resources
    mkdir(outDir.c_str(), 0700);
    for (size_t i=0; i<resources.size(); i++) {
        uint16_t resourceId=resources[i].resourceId;
        uint32_t thisOffset=resources[i].fileOffset;
        uint32_t nextOffset=i==resources.size()-1?is.getSize():resources[i+1].fileOffset;
        uint32_t fileSize=nextOffset-thisOffset;
        
        ByteArray ba=BinaryReader(is, thisOffset, fileSize).readAll();
        uint16_t compressionMagic=ba.size()<2?0:ba[0]|(ba[1]<<8);
        const char * extension="";
        if (compressionMagic==GZIP_MAGIC)
            extension=".gz";
        
        string outFilename=outDir+'/'+std::to_string(resourceId)+extension;
        cout << "Extracting " << outFilename << endl;
        extract(outFilename, ba);
    }
    
    // Create symbolic links for aliases
    // TODO
}

static const TypeRegistration TR("chromium", detect, extract);
