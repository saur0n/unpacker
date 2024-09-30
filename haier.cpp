/*******************************************************************************
 *  Haier firmware unpacker
 *  
 *  © 2022—2024, Sauron <fpsxdump@saur0n.science>
 ******************************************************************************/

#include <iostream>
#include "REUtils.hpp"
#include "TypeRegistration.hpp"

using std::cout;
using std::endl;
using std::string;
using std::vector;

static ByteArray decompressLZSS(const ByteArray &in) {
    ByteArray result;
    uint16_t flags=0;
    size_t inPos=0;
    
    while (inPos<in.size()) {
        flags>>=1;
        
        if (!(flags&0x100)) {
            // get next flags
            flags=0xFF00|in[inPos++];
        }
        
        if (flags&1) {
            // plain byte
            uint8_t c=in[inPos++];
            result.push_back(c);
        }
        else {
            // dictionary bytes
            uint8_t x=in[inPos++];
            uint8_t count=(x>>4)+3;
            uint16_t offset=((x&0x0F)<<8)|in[inPos++];
            
            while (count--) {
                if (size_t(offset)>result.size())
                    result.push_back(0);
                else
                    result.push_back(result[result.size()-offset]);
            }
        }
    }
    
    return result;
}

static void extract(BinaryReader &is, const string &outDir) {
    vector<off_t> segments;
    
    try {
        // Find starts of each segment by looking up for magic number. The
        // method is probabilistic, but for known firmwares it gives correct
        // results.
        BinaryReader temp(is);
        while (temp.available()) {
            off_t offset=temp.tell();
            if (temp.readInt()==0x55AA5AA5)
                segments.push_back(offset);
        }
    }
    catch (EOFException &e) {}
    
    for (size_t i=0; i<segments.size(); i++) {
        cout << "Segment at " << Hex(segments[i]) << endl;
        BinaryReader window(is, segments[i], BinaryReader::END);
        uint32_t magic=window.readInt();
        uint32_t unknown=window.readInt();
        uint32_t length=window.readInt();
        cout << "    Unknown: " << Hex(unknown) << endl;
        cout << "    Length: " << length << endl;
        ByteArray compressedData=window.read(length);
        ByteArray data=decompressLZSS(compressedData);
        
        string filename=outDir+'/'+"seg_"+std::to_string(i)+".seg";
        upp::File out(filename.c_str(), O_WRONLY|O_CREAT|O_TRUNC);
        out.write(data.data(), data.size());
    }
}

TR_NODETECT(haier);
