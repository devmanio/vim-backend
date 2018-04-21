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

template<typename B, typename S> void append_item(B &_base, const S &_st) {
    if ( _base.data.empty() )
        _base.data = _st.account;
    else
        _base.data.append( get_format_account(_st.account, '@') );
}

template<typename B, typename S> void remove_item(B &_base, const S &_st) {
    size_t old_pos = 0;
    std::string &str = _base.data;
    const std::string &str_r = _st.account;

    while( !str.empty() ) {
        auto pos = str.find(str_r, old_pos);
        if (pos == str.npos)
            break;

        auto pos_delimiter_front = pos != 0 ? pos - 1 : 0;
        auto pos_delimiter_back = pos + str_r.length();

        bool is_front = (str.at(pos_delimiter_front) == '@' || pos_delimiter_front == 0);
        bool is_back = false;
        if ( pos_delimiter_back >= str.length() )
            is_back = true;
        else if ( str.at(pos_delimiter_back) == '@' )
            is_back = true;

        if ( is_front && is_back ) {
            str.erase(pos_delimiter_front, str_r.length() + 1);
            break;
        }

        old_pos = pos_delimiter_back;
    }
}

template<typename B, typename S> void append_post_reccord(B &_base, const S &_st) {
    const auto &str = std::to_string(_st.uuid_post) + "$" + _st.url + "$" + _st.hash;
    if ( _base.data.empty() )
        _base.data = str;
    else
        _base.data.append( get_format_account(str, '@') );
}

}
}
