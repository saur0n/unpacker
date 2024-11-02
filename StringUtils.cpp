#include "StringUtils.hpp"

using std::string;
using std::wstring;

/******************************************************************************/

bool endsWith(string const &fullString, string const &ending) {
    if (fullString.length()>=ending.length())
        return (0==fullString.compare(fullString.length()-ending.length(), ending.length(), ending));
    else
        return false;
}

string replaceExtension(const string &filename, const string &extension) {
    size_t index=filename.rfind('.');
    string basename=(index==string::npos)?filename:filename.substr(0, index);
    return basename+'.'+extension;
}

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

string convert(const wstring &in) {
    string result;
    for (size_t i=0; i<in.size(); i++) {
        wchar_t c=in[i];
        if (c<0x80)
            result.push_back(c);
        else if (c<0x800) {
            result.push_back(0xC0|(c>>6));
            result.push_back(0x80|(c&63));
        }
        else if (c<0x8000) {
            result.push_back(0xE0|(c>>12));
            result.push_back(0x80|((c>>6)&63));
            result.push_back(0x80|(c&63));
        }
        else {
            result.push_back(0xF0|(c>>18));
            result.push_back(0x80|((c>>12)&63));
            result.push_back(0x80|((c>>6)&63));
            result.push_back(0x80|(c&63));
        }
    }
    return result;
}
