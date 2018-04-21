/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#include "hackathon.hpp"
#include <eosiolib/math.hpp>

/**
 *  The init() and apply() methods must have C calling convention so that the blockchain can lookup and
 *  call these methods.
 */
extern "C" {

    /// The apply method implements the dispatch of events to this contract
    void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
        eosio::hackathon(receiver).apply( code, action );
    }

} // extern "C"

eosio::hackathon::hackathon(account_name creator)
    : _creator(creator)
{}

bool eosio::hackathon::apply(account_name creator, action_name action)
{
    if( creator != _creator )
       return false;

    switch( action ) {
    case N(init):
        print( "init contract\n" );
        init_contract( unpack_action_data<st_init>() );
        return true;
    case N(addaccount):
        print( "add account in emission table\n" );
        add_account( unpack_action_data<st_account_info>() );
        return true;
    case N(fundaccount):
        print( "fund account\n" );
        fund_account( unpack_action_data<st_account_balance>() );
        return true;
    case N(upvote):
        print( "up vote\n" );
        up_vote( unpack_action_data<st_vote>() );
        return true;
    case N(downvote):
        print( "down vote\n" );
        down_vote( unpack_action_data<st_vote>() );
        return true;
    case N(createpost):
        print( "create post\n" );
        create_post( unpack_action_data<st_post>() );
        return true;
    }

    return false;
}

void eosio::hackathon::add_account(const eosio::objects::st_account_info &m_account)
{
    require_auth( _creator );
    emission_table table( _creator, _creator );
    add_struct_in_table( table, m_account );
    add_account_in_accounts_table(m_account.account);

    update_info(info::add_account);
}

void eosio::hackathon::fund_account(const eosio::objects::st_account_balance &m_account)
{
//    require_auth( m_account.account );
    const auto &account_name = m_account.account;
    accounts_table table(_creator, account_name);
    auto find_iterator = table.find(account_name);
    if ( find_iterator == table.end() ) {
        auto lambda = [&]( auto &item ) {
            item.account = account_name;
            item.balance = m_account.balance;
        };

        table.emplace( _creator, lambda );
        return;
    }

    auto lambda = [&]( auto &item ) {
        item.balance += m_account.balance;
    };

    table.modify( find_iterator, 0, lambda );

}

bool eosio::hackathon::add_account_in_accounts_table(const account_name &m_account)
{
    accounts_table table(_creator, m_account);
    if ( find_account(table, m_account) ) {
        eosio_assert( true, "Such an account exists" );
        return false;
    }

    auto lambda = [&]( auto &item ) {
        item.account = m_account;
        item.balance = asset(0, string_to_symbol(4, "VIM"));
    };

    table.emplace( _creator, lambda );
    return true;
}

template<typename T, typename K>
bool eosio::hackathon::find_account(const T &m_table, const K &m_key)
{
    auto it = m_table.find(m_key);
    if ( it != m_table.end() )
        return true;

    return false;
}

template<typename T, typename S>
void eosio::hackathon::remove_struct_in_table(T &m_table, const S &m_struct)
{
    auto it = m_table.find(m_struct.primary_key());
    if ( it != m_table.end() )
        m_table.erase(it);
}

template<typename T, typename S>
void eosio::hackathon::add_struct_in_table(T &m_table, const S &m_struct)
{
    if ( find_account(m_table, m_struct.primary_key()) )
        eosio_assert( true, "Such an account exists" );

    auto lambda = [&]( auto &item ) {
        item = m_struct;
    };

    m_table.emplace( _creator, lambda );
}

void eosio::hackathon::up_vote(const eosio::objects::st_vote &m_vote)
{
    vote_table table(_creator, _creator);
    st_vote_record post;
    auto it = table.find(m_vote.uuid_post);
    if ( it == table.end() ) {
        post.uuid_post = m_vote.uuid_post;
        it = table.emplace( _creator, [&]( auto& item ) {
            item = post;
        });
    }

    post = *it;
    generator::append_item(post, m_vote);
    table.modify( it, 0, [&]( auto& item ) {
        item = post;
    });
}

void eosio::hackathon::down_vote(const eosio::objects::st_vote &m_vote)
{
    vote_table table(_creator, _creator);
    st_vote_record post;
    auto it = table.find(m_vote.uuid_post);
    if ( it == table.end() )
        return;

    post = *it;
    generator::remove_item(post, m_vote);
    table.modify( it, 0, [&]( auto& item ) {
        item = post;
    });
}

void eosio::hackathon::create_post(const eosio::objects::st_post &m_post)
{
    post_table table(_creator, _creator);
    st_post_record post_record;;
    auto it = table.find(m_post.account);
    if ( it == table.end() ) {
        post_record.account = m_post.account;
        it = table.emplace( _creator, [&]( auto& item ) {
            item = post_record;
        });
    }

    post_record = *it;
    generator::append_post_reccord(post_record, m_post);
    table.modify( it, 0, [&]( auto& item ) {
        item = post_record;
    });
}

void eosio::hackathon::init_contract( const st_init &m_init )
{
    (void)m_init;
    accounts_table table_accounts(_creator, _creator);
    auto find_iterator_accounts = table_accounts.find(_creator);
    if ( find_iterator_accounts == table_accounts.end() ) {
        auto lambda_accounts = [&]( auto &item ) {
            item.account = _creator;
            item.balance = asset(0, string_to_symbol(4, "VIM"));
        };

        table_accounts.emplace( _creator, lambda_accounts );
    }

    info_table table_info(_creator, _creator);
    auto find_iterator_info = table_info.find(_creator);
    if ( find_iterator_info == table_info.end() ) {
        auto lambda_info = [&]( auto &item ) {
            item.key = 0;
            item.count_posts = 0;
            item.count_votes = 0;
            item.count_accounts = 0;
            item.released_tokens = 200000000;
        };

        table_info.emplace( _creator, lambda_info );
    }
}

