#ifndef __STRINGUTILS_HPP
#define __STRINGUTILS_HPP

#include <string>

bool endsWith(std::string const &fullString, std::string const &ending);
std::string replaceExtension(const std::string &filename, const std::string &extension);

#endif
