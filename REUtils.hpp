/*******************************************************************************
 *  FPSX/ROFS unpacking program
 ******************************************************************************/

#ifndef __REUTILS_HPP
#define __REUTILS_HPP

#include <cstdint>
#include <ostream>
#include <unix++/File.hpp>

/** Indicates that an attempt to read beyound of file or a block occurred **/
class EOFException {};

class BinaryReader {
public:
    class ToEnd {};
    BinaryReader(upp::File &file) :
        file(file), start(0), offset(0), size(file.seek(0, SEEK_END)) {}
    BinaryReader(const BinaryReader &other) :
        file(other.file), start(other.start+other.offset), offset(0), size(other.size) {}
    BinaryReader(const BinaryReader &other, size_t length) :
        file(other.file), start(other.start+other.offset), offset(0), size(length) {}
    BinaryReader(const BinaryReader &other, off_t start, size_t length) :
        file(other.file), start(other.start+start), offset(0), size(length) {}
    BinaryReader(const BinaryReader &other, off_t start, ToEnd marker) :
        file(other.file), start(other.start+start), offset(0), size(other.size-start) {}
    virtual ~BinaryReader() {}
    off_t debug() const {
        return start+offset;
    }
    off_t tell() const {
        return offset;
    }
    size_t available() {
        return size-offset;
    }
    bool atEnd(size_t margin=0) const {
        return offset+margin>=size;
    }
    void skip(unsigned bytes) {
        if (offset+bytes>size)
            throw "cannot skip bytes";
        offset+=bytes;
    }
    BinaryReader window(size_t length) {
        if (offset+length>size)
            throw std::string("window is too big: ")+std::to_string(length)+">"+std::to_string(size-offset);
        BinaryReader result(*this, length);
        skip(length);
        return result;
    }
    uint8_t readByte() {
        uint8_t result;
        read(&result, sizeof(result));
        return result;
    }
    uint16_t readShort() {
        return __builtin_bswap16(read<uint16_t>());
    }
    uint16_t readShortLE() {
        return read<uint16_t>();
    }
    uint32_t readInt() {
        return __builtin_bswap32(read<uint32_t>());
    }
    uint32_t readIntLE() {
        return read<uint32_t>();
    }
    uint64_t readLong() {
        return __builtin_bswap64(read<uint64_t>());
    }
    uint64_t readLongLE() {
        return read<uint64_t>();
    }
    std::string readShortString() {
        return readString(readByte());
    }
    std::string readString(size_t length) {
        std::string result(length, '\0');
        read(&result[0], length);
        return result;
    }
    std::string readShortUnicodeString() {
        return readUnicodeString(readByte());
    }
    std::string readUnicodeString(size_t length) {
        std::string result(length, '\0');
        for (unsigned i=0; i<length; i++) {
            result[i]=readByte();
            readByte();
        }
        return result;
    }
    void extract(const std::string &destination, off_t offset, size_t length, bool truncate=false) {
        int flags=O_WRONLY|O_CREAT;
        if (truncate)
            flags|=O_TRUNC;
        upp::File out(destination.c_str(), flags);
        out.seek(offset);
        std::string data(length, '\0');
        read(&data[0], length);
        out.write(data.data(), data.length());
    }
    void extract(const std::string &destination, bool truncate=false) {
        off_t pos=tell();
        extract(destination, pos, size-pos, truncate);
    }
    
protected:
    void read(void * buffer, size_t length) {
        size_t retval=file.read(buffer, length, offset+start);
        offset+=length;
        if (retval<length)
            throw EOFException();
    }
    template <class T>
    T read() {
        T result;
        read(&result, sizeof(result));
        return result;
    }
    
private:
    upp::File &file;
    const off_t start;
    off_t offset;
    size_t size;
};

class Indent {
public:
    Indent() : offset(0) {}
    Indent(const Indent &other) : offset(other.offset+1) {}
    unsigned short getOffset() const { return offset; }
    
private:
    unsigned short offset;
};

inline std::ostream &operator <<(std::ostream &stream, const Indent &indent) {
    for (unsigned short i=0; i<indent.getOffset(); i++)
        stream << "  ";
    return stream;
}

/** Just a syntactic sugar for printing hexadecimal values **/
template <class T=unsigned int>
class Hex {
public:
    Hex(T value) : value(value) {}
    T value;
};

template <class T>
std::ostream &operator <<(std::ostream &stream, Hex<T> value) {
    return stream << std::hex << '0' << 'x' << value.value << std::dec;
}

#endif
