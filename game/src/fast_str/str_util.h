#include "str.h"


namespace fstr {

class Split {
private:
    str m_val;
    char m_pat;
    size_t m_index = 0;

public:
    Split(str val, char pat);

    std::optional<str> next();
};

}

