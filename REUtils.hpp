/*******************************************************************************
 *  FPSX/ROFS unpacking program
 ******************************************************************************/

#ifndef __REUTILS_HPP
#define __REUTILS_HPP

#include <cstdint>
#include <ostream>
#include <unix++/File.hpp>
#include <vector>

/** Vector of bytes **/
using ByteArray=std::vector<uint8_t>;

/** Uncompress zlib-compressed data blob **/
ByteArray uncompress(const ByteArray &in);

/** Indicates that an attempt to read beyound of file or a block occurred **/
class EOFException {};

class BinaryReader {
public:
    enum ToEnd { END };
    BinaryReader(upp::File &file);
    BinaryReader(const BinaryReader &other);
    BinaryReader(const BinaryReader &other, size_t length);
    BinaryReader(const BinaryReader &other, off_t start, size_t length);
    BinaryReader(const BinaryReader &other, off_t start, ToEnd marker);
    virtual ~BinaryReader();
    size_t getSize() const { return size; }
    off_t debug() const;
    off_t tell() const;
    size_t available() const;
    bool atEnd(size_t margin=0) const;
    void skip(unsigned bytes);
    BinaryReader window(size_t length);
    uint8_t readByte();
    uint16_t readShort();
    uint16_t readShortLE();
    uint32_t readInt();
    uint32_t readIntLE();
    uint64_t readLong();
    uint64_t readLongLE();
    std::string readShortString();
    std::string readString(size_t length);
    std::string readShortUnicodeString();
    std::string readUnicodeString(size_t length);
    void extract(const std::string &destination, off_t offset, size_t length, bool truncate=false);
    void extract(const std::string &destination, bool truncate=false);
    ByteArray read(size_t maxLength);
    ByteArray readAll();
    
protected:
    void read(void * buffer, size_t length);
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

/** Convert binary string to a hexadecimal string **/
std::string toHexString(const std::string &in);
/** Remove whitespace characters from the end of the string **/
std::string trim(const std::string &in);

#endif
