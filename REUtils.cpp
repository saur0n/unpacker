/*******************************************************************************
 *  FPSX/ROFS unpacking program
 ******************************************************************************/

#include <cstdio>
#include <zlib.h>
#include "REUtils.hpp"

using std::string;
using std::vector;

/******************************************************************************/

int ZEXPORT uncompress_(Bytef *dest, uLongf *destLen, const Bytef *source, uLong *sourceLen) {
    z_stream stream;
    int err;
    const uInt max = (uInt)-1;
    uLong len, left;
    Byte buf[1];    /* for detection of incomplete stream when *destLen == 0 */

    len = *sourceLen;
    if (*destLen) {
        left = *destLen;
        *destLen = 0;
    }
    else {
        left = 1;
        dest = buf;
    }

    stream.next_in = (z_const Bytef *)source;
    stream.avail_in = 0;
    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;

    err = inflateInit(&stream);
    printf("err1=%d\n", err);
    if (err != Z_OK) return err;

    stream.next_out = dest;
    stream.avail_out = 0;

    do {
        if (stream.avail_out == 0) {
            stream.avail_out = left > (uLong)max ? max : (uInt)left;
            left -= stream.avail_out;
        }
        if (stream.avail_in == 0) {
            stream.avail_in = len > (uLong)max ? max : (uInt)len;
            len -= stream.avail_in;
        }
        err = inflate(&stream, Z_NO_FLUSH);
    } while (err == Z_OK);

    *sourceLen -= len + stream.avail_in;
    if (dest != buf)
        *destLen = stream.total_out;
    else if (stream.total_out && err == Z_BUF_ERROR)
        left = 1;

    inflateEnd(&stream);
    printf("err2=%d\n", err);
    return err == Z_STREAM_END ? Z_OK :
           err == Z_NEED_DICT ? Z_DATA_ERROR  :
           err == Z_BUF_ERROR && left + stream.avail_out ? Z_DATA_ERROR :
           err;
}

ByteArray uncompress(const ByteArray &inData) {
    uLongf outLength=inData.size()*2;
    ByteArray outData;
    for (bool unpacked=false; !unpacked;) {
        outData.resize(outLength);
        size_t inLength=inData.size();
        int retval=uncompress_(&outData[0], &outLength, &inData[0], &inLength);
        if (retval==Z_OK) {
            outData.resize(outLength);
            unpacked=true;
        }
        else if (retval==Z_DATA_ERROR)
            throw "Z_DATA_ERROR";
        else if (retval==Z_MEM_ERROR)
            throw "Z_MEM_ERROR";
        else if (retval==Z_BUF_ERROR)
            outLength+=outLength;
        else
            throw "Z_UNKNOWN_ERROR";
    }
    return outData;
}

/******************************************************************************/

BinaryReader::BinaryReader(upp::File &file) :
    file(file), start(0), offset(0), size(file.seek(0, SEEK_END)) {}

BinaryReader::BinaryReader(const BinaryReader &other) :
    file(other.file), start(other.start+other.offset), offset(0), size(other.size) {}

BinaryReader::BinaryReader(const BinaryReader &other, size_t length) :
    file(other.file), start(other.start+other.offset), offset(0), size(length) {}

BinaryReader::BinaryReader(const BinaryReader &other, off_t start, size_t length) :
    file(other.file), start(other.start+start), offset(0), size(length) {}

BinaryReader::BinaryReader(const BinaryReader &other, off_t start, ToEnd marker) :
    file(other.file), start(other.start+start), offset(0), size(other.size-start) {}

BinaryReader::~BinaryReader() {}

off_t BinaryReader::debug() const {
    return start+offset;
}

off_t BinaryReader::tell() const {
    return offset;
}

size_t BinaryReader::available() const {
    return size-offset;
}

bool BinaryReader::atEnd(size_t margin) const {
    return offset+margin>=size;
}

void BinaryReader::skip(unsigned bytes) {
    if (offset+bytes>size)
        throw "cannot skip bytes";
    offset+=bytes;
}

BinaryReader BinaryReader::window(size_t length) {
    if (offset>size)
        throw "cannot create a window, because the cursor is beyond the end";
    if (offset+length>size)
        throw std::string("window is too big: ")+std::to_string(length)+">"+std::to_string(size-offset);
    BinaryReader result(*this, length);
    skip(length);
    return result;
}

uint8_t BinaryReader::readByte() {
    uint8_t result;
    read(&result, sizeof(result));
    return result;
}

uint16_t BinaryReader::readShort() {
    return __builtin_bswap16(read<uint16_t>());
}

uint16_t BinaryReader::readShortLE() {
    return read<uint16_t>();
}

uint32_t BinaryReader::readInt() {
    return __builtin_bswap32(read<uint32_t>());
}

uint32_t BinaryReader::readIntLE() {
    return read<uint32_t>();
}

uint64_t BinaryReader::readLong() {
    return __builtin_bswap64(read<uint64_t>());
}

uint64_t BinaryReader::readLongLE() {
    return read<uint64_t>();
}

string BinaryReader::readShortString() {
    return readString(readByte());
}

string BinaryReader::readString(size_t length) {
    string result(length, '\0');
    read(&result[0], length);
    return result;
}

string BinaryReader::readShortUnicodeString() {
    return readUnicodeString(readByte());
}

string BinaryReader::readUnicodeString(size_t length) {
    string result(length, '\0');
    for (unsigned i=0; i<length; i++) {
        result[i]=readByte();
        readByte();
    }
    return result;
}

void BinaryReader::extract(const string &destination, off_t offset, size_t length, bool truncate) {
    int flags=O_WRONLY|O_CREAT;
    if (truncate)
        flags|=O_TRUNC;
    upp::File out(destination.c_str(), flags);
    out.seek(offset);
    string data(length, '\0');
    read(&data[0], length);
    out.write(data.data(), data.length());
}

void BinaryReader::extract(const string &destination, bool truncate) {
    off_t pos=tell();
    extract(destination, pos, size-pos, truncate);
}

ByteArray BinaryReader::read(size_t maxLength) {
    ByteArray result(maxLength);
    size_t nRead=file.read(&result[0], maxLength, offset+start);
    offset+=nRead;
    result.resize(nRead);
    return result;
}

ByteArray BinaryReader::readAll() {
    return read(available());
}

void BinaryReader::read(void * buffer, size_t length) {
    if (length) {
        size_t retval=file.read(buffer, length, offset+start);
        offset+=retval;
        if (retval<length)
            throw EOFException();
    }
}

/******************************************************************************/

string toHexString(const string &in) {
    static const char HEXCHARS[]="0123456789ABCDEF";
    string result(in.length()*2, '\0');
    for (size_t i=0; i<in.size(); i++) {
        unsigned char c=in[i];
        result[i*2+0]=HEXCHARS[c>>4];
        result[i*2+1]=HEXCHARS[c&15];
    }
    return result;
}

string trim(const string &str) {
    size_t length=str.length();
    while ((length>0)&&(str[length-1]=='\0'))
        length--;
    return str.substr(0, length);
}

