#include "StringUtils.hpp"

using std::string;

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
