#include <set>
#include "REUtils.hpp"
#include "TypeRegistration.hpp"

using std::set;
using std::string;
using std::vector;

/******************************************************************************/

static set<const TypeRegistration *> &getRegistrations() {
    static set<const TypeRegistration *> list;
    return list;
}

TypeRegistration::TypeRegistration(const char * name, DetectFunction detect, ExtractFunction extract) :
        name(name), detect(detect), extract(extract) {
    getRegistrations().insert(this);
}

TypeRegistration::~TypeRegistration() {
    getRegistrations().erase(this);
}

vector<const char *> TypeRegistration::list() {
    vector<const char *> result;
    auto &registrations=getRegistrations();
    
    for (auto i=registrations.begin(); i!=registrations.end(); ++i)
        result.emplace_back((*i)->name);
    
    return result;
}

TypeRegistration::ExtractFunction TypeRegistration::resolve(BinaryReader &is, const string &filename) {
    auto &registrations=getRegistrations();
    
    for (auto i=registrations.begin(); i!=registrations.end(); ++i) {
        // `magicReader` is called magic not because it is magic itself, but
        // because it is intended for reading magic numbers
        BinaryReader magicReader(is);
        if ((*i)->detect(magicReader, filename))
            return (*i)->extract;
    }
    
    return nullptr;
}
