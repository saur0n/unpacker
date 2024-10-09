/*******************************************************************************
 *  Unpacks all images from a firmware dump
 *  
 *  © 2024, Sauron <fpsxdump@saur0n.science>
 ******************************************************************************/

#include <iostream>
#include <unix++/File.hpp>
#include "REUtils.hpp"
#include "TypeRegistration.hpp"

using std::cout;
using std::endl;
using std::string;
using upp::File;

class StreamReader {
public:
    StreamReader(const ByteArray &ba, size_t offset) :
        ba(ba), offset(offset) {}
    uint8_t getByte() {
        if (offset>=ba.size())
            throw false;
        else
            return ba[offset++];
    }
    uint16_t getShort() {
        if (offset+1>=ba.size())
            throw false;
        else {
            uint16_t result=(ba[offset]<<8)|ba[offset+1];
            offset+=2;
            return result;
        }
    }
    void skip(uint16_t length) {
        if (offset+length>ba.size())
            throw false;
        else
            offset+=length;
    }
    size_t getOffset() const { return offset; }
    
private:
    const ByteArray &ba;
    size_t offset;
};

static size_t detectJPEG(const ByteArray &data, size_t offset) {
    try {
        StreamReader sr(data, offset);
        
        if (sr.getByte()!=0xFF)
            return string::npos;
        if (sr.getByte()!=0xD8)
            return string::npos;
        
        enum { START, MARKER, SCAN } state=START;
        size_t result=string::npos;
        
        for (bool end=false; !end;) {
            uint8_t byte=sr.getByte();
            
            if (state==START) {
                if (byte!=0xFF)
                    end=true;
                else
                    state=MARKER;
            }
            else if (state==MARKER) {
                uint16_t length;
                
                switch (byte) {
                case 0xC0:
                case 0xC4:
                case 0xDB:
                case 0xE0:
                case 0xE1:
                case 0xE2:
                case 0xE3:
                case 0xE4:
                case 0xE5:
                case 0xE6:
                case 0xE7:
                case 0xE8:
                case 0xE9:
                case 0xEA:
                case 0xEB:
                case 0xEC:
                case 0xED:
                case 0xEE:
                case 0xEF:
                    length=sr.getShort();
                    if (length<2)
                        end=true;
                    else
                        sr.skip(length-2);
                    state=START;
                    break;
                case 0xD0:
                case 0xD1:
                case 0xD2:
                case 0xD3:
                case 0xD4:
                case 0xD5:
                case 0xD6:
                case 0xD7:
                    state=START;
                    break;
                case 0x00:
                case 0xDA:
                    state=SCAN;
                    break;
                case 0xD9:
                    result=sr.getOffset()-offset;
                    end=true;
                    break;
                case 0xDD:
                    sr.skip(4);
                    state=START;
                    break;
                default:
                    cout << "[JPEG] unknown chunk " << std::hex << unsigned(byte) << endl;
                    end=true;
                }
            }
            else if (state==SCAN) {
                if (byte==0xFF)
                    state=MARKER;
            }
        }
        
        return result;
    }
    catch (bool) {
        return string::npos;
    }
}

static void extract(BinaryReader &is, const string &outDir) {
    ByteArray data=is.readAll();
    
    for (size_t offset=0; offset<data.size(); offset++) {
        size_t length=detectJPEG(data, offset);
        if (length!=string::npos) {
            cout << "[JPEG] found at " << offset << endl;
            
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "0x%06zX", offset);
            
            string filename=outDir+'/'+buffer+".jpeg";
            File outFile(filename.c_str(), O_WRONLY|O_TRUNC|O_CREAT);
            outFile.write(&data[offset], length);
        }
    }
}

TR_NODETECT(images);