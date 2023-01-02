/*******************************************************************************
 *  Haier firmware unpacker
 *  
 *  © 2022, Sauron <fpsxdump@saur0n.science>
 ******************************************************************************/

#include <iostream>
#include "REUtils.hpp"

using std::cout;
using std::endl;
using std::vector;

#define N         4096  /* size of ring buffer - must be power of 2 */
#define F         18    /* upper limit for match_length */
#define THRESHOLD 2     /* encode string into position and length
                           if match_length is greater than this */
#define NIL       N     /* index for root of binary search trees */

ByteArray decompressLZSS(const ByteArray &in) {
    /* ring buffer of size N, with extra F-1 bytes to aid string comparison */
    const uint8_t * src=&in[0];
    uint8_t text_buf[N + F - 1];
    const uint8_t * srcend = src + in.size();
    int  i, j, k, r, c;
    unsigned int flags;
    ByteArray result;
    
    for (i = 0; i < N - F; i++)
        text_buf[i] = 0;
    r = N - F;
    flags = 0;
    for ( ; ; ) {
        if (((flags >>= 1) & 0x100) == 0) {
            if (src < srcend) c = *src++; else break;
            //cout << "h: " << hex << unsigned(c) << dec << endl;
            flags = c | 0xFF00;  /* uses higher byte cleverly */
        }   /* to count eight */
        if (flags & 1) {
            if (src < srcend) c = *src++; else break;
            result.push_back(c);
            text_buf[r++] = c;
            r &= (N - 1);
        }
        else {
            if (src < srcend) j = *src++; else break;
            if (src < srcend) i = *src++; else break;
            //cout << "ref " << hex << i << " " << j << dec << "\n";
            i |= ((j & 0x0F) << 8);
            j  =  (j >> 4) + THRESHOLD;
            for (k = 0; k <= j; k++) {
                c = text_buf[(i + k) & (N - 1)];
                result.push_back(c);
                text_buf[r++] = c;
                r &= (N - 1);
            }
        }
    }
    
    return result;
}

void extractHaierFirmware(BinaryReader &is) {
    vector<off_t> segments;
    
    try {
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
        ByteArray compressedData=window.read(length);
        ByteArray data=decompressLZSS(compressedData);
        
        upp::File out(("seg_"+std::to_string(i)+".seg").c_str(), O_WRONLY|O_CREAT|O_TRUNC);
        out.write(data.data(), data.size());
    }
}
