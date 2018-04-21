/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#include "structures.hpp"


namespace eosio {
    using namespace objects;

enum class info {
    up_vote,
    down_vote,
    add_account,
    create_post
};


class hackathon
{
public:
    hackathon(account_name creator);
    bool apply( account_name creator, action_name action );

private:
    void add_account( const st_account_info &m_account );
    void fund_account( const st_account_balance &m_account );

    void up_vote( const st_vote &m_vote );
    void down_vote( const st_vote &m_vote );

    void create_post( const st_post &m_post );

    void init_contract(const st_init &m_init);

private: //TODO auxiliary methods
    template <typename T, typename K> // TODO T - table, K - key in table
    bool find_account(const T &m_table, const K &m_key);

    template<typename T, typename S> // TODO T - table, S - struct stored in table
    void add_struct_in_table( T &m_table, const S &m_struct );

    template<typename T, typename S> // TODO T - table, S - struct stored in table
    void remove_struct_in_table( T &m_table, const S &m_struct );

    bool add_account_in_accounts_table( const account_name &m_account ); // TODO add account in account table

private:
    account_name _creator;
};

typedef eosio::multi_index<N(infotable),  st_info>                info_table;
typedef eosio::multi_index<N(votetable),  st_vote_record>         vote_table;
typedef eosio::multi_index<N(posttable),  st_post_record>         post_table;
typedef eosio::multi_index<N(accounts),   st_account_balance> accounts_table;
typedef eosio::multi_index<N(emisstable), st_account_info>    emission_table;
}

