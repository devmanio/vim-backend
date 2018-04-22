#include "generator.hpp"

namespace eosio {
namespace objects {

using std::string;
using std::array;

struct st_transfer
{
    account_name from;
    account_name to;
    asset        quantity;

    EOSLIB_SERIALIZE( st_transfer, (from)(to)(quantity) )
};

struct st_account_info {
    account_name account;

    uint64_t primary_key() const {
        return account;
    }
    EOSLIB_SERIALIZE( st_account_info, (account) )
};

struct st_hash_account : public st_account_info  {
    int64_t hash;

    EOSLIB_SERIALIZE( st_hash_account, (account)(hash) )
};

struct st_account_balance : public st_account_info {
    asset balance;

    EOSLIB_SERIALIZE( st_account_balance, (account)(balance) )
};

struct st_emission
{
    int64_t hash;
    EOSLIB_SERIALIZE( st_emission, (hash) )
};

struct st_fund_account_steem
{
    asset amount;
    int64_t hash;
    EOSLIB_SERIALIZE( st_fund_account_steem, (amount)(hash) )
};

struct st_vote
{
    uint64_t uuid_post;
    string account;
    EOSLIB_SERIALIZE( st_vote, (uuid_post)(account) )
};

struct st_post
{
    account_name account;
    uint64_t uuid_post;
    string url;
    string hash;

    EOSLIB_SERIALIZE( st_post, (account)(uuid_post)(url)(hash) )
};

struct st_vote_record
{
    uint64_t uuid_post;
    string data;
    asset cost;

    uint64_t primary_key() const {
        return uuid_post;
    }

    EOSLIB_SERIALIZE( st_vote_record, (uuid_post)(data)(cost) )
};

struct st_post_record
{
    account_name account;
    string data;

    account_name primary_key() const {
        return account;
    }

    EOSLIB_SERIALIZE( st_post_record, (account)(data) )
};

struct st_info
{
    uint8_t key;
    uint64_t count_posts;
    uint64_t count_votes;
    uint64_t count_accounts;
    uint64_t released_tokens;

    uint8_t primary_key() const {
        return key;
    }

    EOSLIB_SERIALIZE( st_info, (key)(count_posts)(count_votes)(count_accounts)(released_tokens) )
};

struct st_init
{
    uint8_t autoruzation;
    EOSLIB_SERIALIZE( st_init, (autoruzation) )
};

}
}
