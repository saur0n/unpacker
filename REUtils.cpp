#include "REUtils.hpp"

using std::string;
using std::vector;

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

vector<uint8_t> BinaryReader::read(size_t maxLength) {
    vector<uint8_t> result(maxLength);
    size_t nRead=file.read(&result[0], maxLength, offset+start);
    offset+=nRead;
    result.resize(nRead);
    return result;
}

vector<uint8_t> BinaryReader::readAll() {
    return read(available());
}

void BinaryReader::read(void * buffer, size_t length) {
    size_t retval=file.read(buffer, length, offset+start);
    offset+=retval;
    if (retval<length)
        throw EOFException();
}
