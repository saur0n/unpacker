/*******************************************************************************
 *  ArchiCAD library cache file unpacker
 *  
 *  © 2025, Sauron <unpacker@saur0n.science>
 ******************************************************************************/

#include <iostream>
#include "REUtils.hpp"
#include "StringUtils.hpp"
#include "TypeRegistration.hpp"

using std::cout;
using std::endl;
using std::string;
using std::vector;

static void extractObject(BinaryReader &is, const string &dir, Indent indent);

static bool detect(BinaryReader &is, const string &filename) {
    return is.readString(4)=="GLCF";
}

static void extractFile(BinaryReader &is, const string &dir, Indent indent) {
    for (unsigned i=0; i<8; i++)
        is.readInt();
    
    uint32_t mimeTypeLength=is.readInt();
    
    for (unsigned i=0; i<6; i++)
        is.readInt();
    
    uint32_t length=is.readInt();
    is.readInt();
    is.readInt();
    is.readInt();
    
    cout << indent << "[FILE] " << is.readUnicodeString(is.readInt()) << endl;
    if (mimeTypeLength)
        cout << indent << "\tType: " << is.readString(mimeTypeLength) << endl;
    is.skip(length);
}

static void extractDirectory(BinaryReader &is, const string &dir, Indent indent) {
    is.readInt();
    is.readInt();
    uint32_t count=is.readInt();
    is.readInt();
    is.readInt();
    is.readInt();
    is.readInt();
    is.readInt();
    uint32_t count2=is.readInt();
    if (count2!=count)
        cout << indent << "! " << count << "!=" << count2 << endl;
    
    for (unsigned i=0; i<3; i++)
        is.readInt();
    
    cout << indent << "[DIR] " << is.readUnicodeString(is.readInt()) << endl;
    for (unsigned i=0; i<count; i++)
        extractObject(is, dir, indent);
}

static void extractObject(BinaryReader &is, const string &dir, Indent indent) {
    uint32_t type=is.readInt();
    if (type==0x464c4452)
        extractDirectory(is, dir, indent);
    else if (type==0x46494c45)
        extractFile(is, dir, indent);
    else
        throw "unknown type";
}

static void extract(BinaryReader &is, const string &dir) {
    (void)dir;
    
    if (is.readString(4)!="GLCF")
        throw "bad magic";
    
    is.readInt();
    is.readShort();
    is.readShort();
    is.readInt();
    
    is.readInt();
    is.readInt();
    is.readInt();
    is.readInt();
    is.readInt();
    uint32_t count=is.readInt();
    is.readInt();
    
    for (unsigned i=0; i<count; i++)
        extractObject(is, dir, Indent());
    
    if (!is.atEnd())
        cout << "Something went wrong: file was not read until the end" << endl;
}

TR(lcf);