#ifndef __TYPEREGISTRATION_HPP
#define __TYPEREGISTRATION_HPP

#include <string>
#include <vector>

class BinaryReader;

/**/
class TypeRegistration {
public:
    using DetectFunction=bool(*)(BinaryReader &is, const std::string &filename);
    using ExtractFunction=void(*)(BinaryReader &is, const std::string &outputDir);
    /** Add a file type **/
    TypeRegistration(const char * name, DetectFunction detect, ExtractFunction extract);
    /** Unregister this file type **/
    ~TypeRegistration();
    /** List the file types **/
    static std::vector<const char *> list();
    /** Get the extracter by its name **/
    static ExtractFunction get(const std::string &name);
    /** Detect the type of file **/
    static ExtractFunction resolve(BinaryReader &is, const std::string &filename);
    /** Always returns false **/
    static bool no(BinaryReader &is, const std::string &filename) { return false; }
    
private:
    explicit TypeRegistration(const TypeRegistration &other)=delete;
    TypeRegistration &operator =(const TypeRegistration &other)=delete;
    
    const char * name;
    DetectFunction detect;
    ExtractFunction extract;
};

#define TR(name) \
    static const TypeRegistration _##name##_tr(#name, detect, extract)

#endif
