

namespace shmy { namespace core {

template<typename T> class Option;


template<typename T>
class Iterator {
public:
    virtual Option<T> next() = 0;
};

} }

