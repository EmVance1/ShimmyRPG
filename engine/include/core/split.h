#include "iter.h"
#include "option.h"
#include "str.h"
#include <string>


namespace shmy { namespace core {


template<typename T> class Option;


class Split : public Iterator<const str> {
private:
    const str m_val;
    char m_pat;
    size_t m_index = 0;
    bool m_stop = false;

public:
    Split(const str val, char pat);
    Split(const String& val, char pat);
    Split(const std::string& val, char pat);

    Option<const str> next();
};


// Split split(const std::string& val, char pat);
std::vector<std::string> split(const std::string& val, char pat);
std::string trim(const std::string& val);


} }

