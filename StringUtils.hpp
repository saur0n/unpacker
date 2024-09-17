#ifndef __STRINGUTILS_HPP
#define __STRINGUTILS_HPP

#include <string>

bool endsWith(std::string const &fullString, std::string const &ending);
std::string replaceExtension(const std::string &filename, const std::string &extension);
/** Convert binary string to a hexadecimal string **/
std::string toHexString(const std::string &in);
/** Remove whitespace characters from the end of the string **/
std::string trim(const std::string &in);

#endif
