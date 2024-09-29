/*******************************************************************************
 *  Akuvox Intercom firmware unpacker
 *  
 *  © 2021—2024, Sauron <fpsxdump@saur0n.science>
 ******************************************************************************/

#include <cstring>
#include <iostream>
#include <openssl/evp.h>
#include <optional>
#include <unix++/FileSystem.hpp>
#include "REUtils.hpp"
#include "StringUtils.hpp"
#include "TypeRegistration.hpp"

using std::cout;
using std::endl;
using std::string;
using std::vector;

class CipherContext {
public:
    CipherContext() : ctx(EVP_CIPHER_CTX_new()) {}
    ~CipherContext() {
        EVP_CIPHER_CTX_free(ctx);
    }
    void decryptInit(const EVP_CIPHER * type, ENGINE * impl, const unsigned char * key, const unsigned char * iv) {
        check(EVP_DecryptInit_ex(ctx, type, impl, key, iv), "EVP_DecryptInit_ex()");
    }
    void setPadding(int padding) {
        check(EVP_CIPHER_CTX_set_padding(ctx, 0), "EVP_CIPHER_CTX_set_padding()");
    }
    void decryptUpdate(unsigned char * out, int &outl, const unsigned char * in, int inl) {
        check(EVP_DecryptUpdate(ctx, out, &outl, in, inl), "EVP_DecryptUpdate()");
    }
    void decryptFinal(unsigned char * outm, int &outl) {
        check(EVP_DecryptFinal_ex(ctx, outm, &outl), "EVP_DecryptFinal_ex()");
    }
    
private:
    static void check(int retval, const char * function) {
        if (retval!=1)
            throw function;
    }
    
    EVP_CIPHER_CTX * ctx;
};

struct SectionType {
    unsigned type;
    const char * description;
    bool encrypted;
    bool compressed;
};

static const SectionType SECTION_TYPES[]={
    {0,     "flash partition",                  true,   false},
    {1,     "upgrader tool",                    false,  false},
    {3,     "config modify",                    true,   false},
    {4,     "system.img",                       true,   false},
    {6,     "system.img (md5)",                 false,  false},
    {7,     "bootlogo.fex",                     true,   false},
    {8,     "peripheral controller",            true,   false},
    {9,     "bootloader.fex",                   true,   false},
    {10,    "resource configuration.md5",       false,  false},
    {11,    "resource configuration.tar.bz2",   true,   false},
    {200,   "mtd0",                             true,   true},
    {201,   "mtd1",                             true,   true},
    {202,   "mtd2",                             true,   true},
    {203,   "mtd3",                             true,   true},
    {204,   "mtd4",                             true,   true},
    {205,   "mtd5",                             true,   true},
    {206,   "mtd6",                             true,   true},
    {220,   "mtd0.md5",                         false,  false},
    {221,   "mtd1.md5",                         false,  false},
    {222,   "mtd2.md5",                         false,  false},
    {223,   "mtd3.md5",                         false,  false},
    {224,   "mtd4.md5",                         false,  false},
    {225,   "mtd5.md5",                         false,  false},
    {226,   "mtd6.md5",                         false,  false},
};

static const unsigned char ENCRYPTION_KEY[]="d3JpdGVfdXBncmFkZXJfYmluX3RvX2Zq";

static string formatVersion(uint32_t version) {
    string result;
    for (unsigned i=0; i<4; i++) {
        if (i!=0)
            result+='.';
        result+=std::to_string((version>>((3-i)*8))&0xFF);
    }
    return result;
}

static const SectionType &getSectionType(unsigned type) {
    static const SectionType DEFAULT {0, nullptr, false, false};
    for (auto i=std::begin(SECTION_TYPES); i!=std::end(SECTION_TYPES); ++i)
        if (i->type==type)
            return *i;
    return DEFAULT;
}

static void uncompressTo(const string &in, upp::File &output) {
    // Read the input file
    upp::File input(in.c_str(), O_RDONLY);
    size_t inLength=input.seek(0, SEEK_END);
    input.seek(0);
    ByteArray inData(inLength);
    input.read(&inData[0], inData.size());
    
    // Decompress the data
    ByteArray outData=uncompress(inData);
    
    // Write the decompressed file
    output.write(outData.data(), outData.size());
}

static bool detect(BinaryReader &is, const string &filename) {
    return is.readString(4)=="MORR";
}

static void extract(BinaryReader &is, const string &dir) {
    // Read main header
    string magic=is.readString(4);
    if (magic!="MORR")
        throw "wrong magic (expected 'MORR')";
    uint32_t headerSize=is.readIntLE();
    uint32_t headerCRC=is.readIntLE();
    BinaryReader header=is.window(headerSize);
    uint32_t type=header.readIntLE();
    uint32_t processType=header.readIntLE();
    uint32_t sections=header.readIntLE();
    uint32_t deviceId=header.readIntLE();
    uint32_t oemId=header.readIntLE();
    uint32_t romVersion=header.readIntLE();
    uint32_t romSize=header.readIntLE();
    uint32_t romChecksum=header.readIntLE();
    header.skip(16u);
    uint32_t midVersion=header.readIntLE();
    uint32_t swProtect=header.readIntLE();
    header.skip(52u);
    uint32_t encryptionType=header.readIntLE();
    
    cout << "Type: " << type << endl;
    cout << "Process type: " << processType << endl;
    cout << "Number of sections: " << sections << endl;
    cout << "Device ID: " << deviceId << endl;
    cout << "OEM ID: " << oemId << endl;
    cout << "ROM version: " << formatVersion(romVersion) << endl;
    cout << "ROM size: " << romSize << endl;
    cout << "ROM checksum: " << romChecksum << endl;
    cout << "MID version: " << midVersion << endl;
    cout << "SW protect: " << swProtect << endl;
    cout << "Encryption type: " << encryptionType << endl;
    
    // Uncompressed image
    std::optional<upp::File> image;
    
    for (unsigned i=0; i<sections; i++) {
        string sectionMagic=is.readString(4);
        if (sectionMagic!="TAPR")
            throw "wrong section magic (expected 'TAPR')";
        uint32_t size=is.readIntLE();
        uint32_t crc=is.readIntLE();
        BinaryReader window=is.window(size);
        uint32_t sectionType=window.readIntLE();
        uint32_t processType=window.readIntLE();
        uint32_t dataType=window.readIntLE();
        uint32_t id=window.readIntLE();
        uint32_t version=window.readIntLE();
        uint32_t dataLength=window.readIntLE();
        uint32_t dataCRC=window.readIntLE();
        uint32_t dataOffset=window.readIntLE();
        
        const SectionType &st=getSectionType(sectionType);
        string sectionTypeStr=st.description?st.description:std::to_string(sectionType);
        
        cout << "Section" << endl;
        cout << "    Section type: " << sectionTypeStr << endl;
        cout << "    Process type: " << processType << endl;
        cout << "    Data type: " << dataType << endl;
        cout << "    ID: " << id << endl;
        cout << "    Version: " << formatVersion(version) << endl;
        cout << "    Data length: " << dataLength << endl;
        cout << "    Checksum: " << dataCRC << endl;
        cout << "    Offset: " << dataOffset << endl;
        
        BinaryReader data(is, dataOffset, dataLength);
        mkdir(dir.c_str(), 0700);
        string filename=dir+'/'+std::to_string(i)+"_"+sectionTypeStr;
        if (st.encrypted&&(encryptionType==1)) {
            // First 1KB of a section is encrypted
            upp::File out(filename.c_str(), O_WRONLY|O_CREAT|O_TRUNC);
            vector<uint8_t> ciphertext;
            
            ciphertext=data.read(1024);
            if (ciphertext.size()<1024)
                out.write(ciphertext.data(), ciphertext.size());
            else {
                uint8_t iv[16];
                memset(iv, 0x30, sizeof(iv));
                
                vector<uint8_t> plaintext(ciphertext.size());
                int p_len=int(plaintext.size()), f_len=0;
                
                CipherContext context;
                context.decryptInit(EVP_aes_128_cbc(), nullptr, ENCRYPTION_KEY+8, iv);
                context.setPadding(0);
                context.decryptUpdate(&plaintext[0], p_len, &ciphertext[0], ciphertext.size());
                context.decryptFinal(&plaintext[p_len], f_len);
                out.write(plaintext.data(), plaintext.size());
                
                ciphertext=data.readAll();
                if (!ciphertext.empty())
                    out.write(ciphertext.data(), ciphertext.size());
            }
            cout << "    Decrypted and exported to " << filename << endl;
        }
        else {
            data.extract(filename);
            cout << "    Exported to " << filename << endl;
        }
        
        /*if (st.compressed) {
            string unFilename=filename+".uncompressed";
            uncompressFile(filename, unFilename);
            cout << "    Uncompressed to " << unFilename << endl;
        }*/
        
        if (st.compressed) {
            if (!image)
                image.emplace("akuvox/mtd", O_CREAT|O_WRONLY|O_TRUNC);
            uncompressTo(filename, *image);
        }
    }
}

TR(akuvox);
