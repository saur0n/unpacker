#include "StringUtils.hpp"

bool endsWith(std::string const &fullString, std::string const &ending) {
    if (fullString.length()>=ending.length())
        return (0==fullString.compare(fullString.length()-ending.length(), ending.length(), ending));
    else
        return false;
}
