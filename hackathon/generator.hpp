#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>

#include <sstream>


namespace eosio {
namespace generator {
const std::string get_format_account(const std::string &_name, const char delimiter) {
    const std::string account = delimiter + _name;
    return account;
}

template<typename T> void split(const std::string &str, const char delimiter, T &container) {
    size_t old_pos = 0;
    while ( !str.empty() ) {
        auto pos = str.find(delimiter, old_pos);
        container.push_back( str.substr(old_pos, pos - old_pos) );

        old_pos = pos + 1;
        if (pos == str.npos)
            break;
    }
}

template<typename T> void join(T &container, const char delimiter, std::string &str) {
    for (auto it = container.begin(); it != container.end(); ++it) {
        if (str.empty())
            str = it->data();
        else
            str.append( get_format_account(it->data(), delimiter) );
    }
}

}
}
