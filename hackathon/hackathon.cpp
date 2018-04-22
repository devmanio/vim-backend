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
        print( "add account in emission table" );
        add_account( unpack_action_data<st_account_info>() );
        return true;
    case N(emission):
        print( "emission" );
        emission( unpack_action_data<st_emission>() );
        return true;
    case N(upvote):
        print( "up vote" );
        up_vote( unpack_action_data<st_vote>() );
        return true;
    case N(downvote):
        print( "down vote" );
        down_vote( unpack_action_data<st_vote>() );
        return true;
    case N(createpost):
        print( "create post" );
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

void eosio::hackathon::emission(const eosio::objects::st_emission &m_emission)
{
    (void) m_emission;
    require_auth( _creator );

    info_table table_info(_creator, _creator);
    auto it_info = table_info.find(0);
    if (it_info == table_info.end())
        eosio_assert(false, "Error: not base account");
    auto info = *it_info;

    auto tokens = double_to_i64(tokens_in_block(info));
    table_info.modify(it_info, 0, [&]( auto &item ) {
        item.released_tokens = info.released_tokens;
    });

    st_account_balance account;
    account.account = _creator;
    account.balance = asset(tokens, string_to_symbol(4, "VIM"));
    fund_account(account);

    emission_table table_emission(_creator, _creator);
    uint64_t per25_one = 0;
    if (info.count_accounts > 0)
        per25_one = double_div((tokens*25/100), info.count_accounts);
    for ( auto it = table_emission.begin(); it != table_emission.end(); ++it ) {
        transfer( st_transfer{_creator, it->account, asset(double_to_i64(per25_one), string_to_symbol(4, "VIM"))} );
    }
}

void eosio::hackathon::transfer(const eosio::objects::st_transfer &m_transfer)
{
    require_auth( m_transfer.from );

    if( !sub_balance(m_transfer.from, m_transfer.quantity) ) {
        eosio_assert(true, "Insufficient funds on the balance sheet");
        return;
    }
    add_balance(m_transfer.to, m_transfer.quantity);
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

void eosio::hackathon::inline_transfer(const st_transfer &m_transfer)
{
    action tranfer_action( permission_level( m_transfer.from, N(active) ), _creator, N(transfer), m_transfer );
    tranfer_action.send();
}

bool eosio::hackathon::add_balance(const account_name &m_name_account, const eosio::asset &m_amount)
{
    accounts_table table(_creator, m_name_account);
    auto it_account = table.find(m_name_account);
    if ( it_account == table.end() ) {
        eosio_assert(true, "This account does not exist");
        return false;
    }

    auto lambda = [&]( auto &item ) {
        item.balance += m_amount;
    };

    table.modify( it_account, 0, lambda );
    return true;
}

bool eosio::hackathon::sub_balance(const account_name &m_name_account, const eosio::asset &m_amount)
{
    accounts_table table(_creator, m_name_account);
    auto it_account = table.find(m_name_account);
    if ( it_account == table.end() ) {
        eosio_assert(false, "This account does not exist");
        return false;
    }

    auto difference_asset = it_account->balance - m_amount;
    if ( (difference_asset.amount < 0) ) { //TODO check on negative balance
        eosio_assert(false, "Error: negative balance. Top up balance.");
        return false;
    }

    auto lambda = [&]( auto &item ) {
        item.balance -= m_amount;
    };

    table.modify( it_account, 0, lambda );
    return true;
}

uint64_t eosio::hackathon::tokens_in_block(st_info &m_info)
{
    auto tokens_in_year = m_info.released_tokens;
    const auto current_year = 2018;

    int days_in_year = 365 + ((current_year%4 == 0 && current_year%100 != 0) || (current_year % 400 == 0));
    int seconds_in_year = 60*60*24*days_in_year;
    int blocks_in_year = seconds_in_year*2;

    auto tokens_in_block = double_div(tokens_in_year, blocks_in_year);
    m_info.released_tokens += double_to_i64(tokens_in_block);

    return tokens_in_block;
}

void eosio::hackathon::update_info(info m_info)
{
    info_table table(_creator, _creator);
    auto it = table.find(0);
    if (it == table.end()) {
        eosio_assert(false, "Error: not record");
        return;
    }
    auto record = *it;

    switch (m_info) {
    case info::up_vote: {
        auto lambda_up_vote = [&]( auto &item ) {
            item = record;
            item.count_votes = record.count_votes + 1;
        };

        table.modify(it, 0, lambda_up_vote);
        break;
    }
    case info::down_vote: {
        auto lambda_down_vote = [&]( auto &item ) {
            item = record;
            item.count_votes = record.count_votes - 1;
        };

        table.modify(it, 0, lambda_down_vote);
        break;
    }
    case info::add_account: {
        auto lambda_down_vote = [&]( auto &item ) {
            item = record;
            item.count_accounts = record.count_accounts + 1;
        };

        table.modify(it, 0, lambda_down_vote);
        break;
    }
    case info::create_post: {
        auto lambda_create_post = [&]( auto &item ) {
            item = record;
            item.count_posts = record.count_posts + 1;
        };

        table.modify(it, 0, lambda_create_post);
        break;
    }
    }
}

void eosio::hackathon::calculation_cost_posts()
{
    accounts_table table_accounts(_creator, _creator);
    auto it = table_accounts.find(_creator);
    if (it == table_accounts.end())
        eosio_assert(false, "Error: not base account");
    auto base_account = *it;

    info_table table_info(_creator, _creator);
    auto it_info = table_info.find(0);
    if (it_info == table_info.end())
        eosio_assert(false, "Error: not base account");
    auto info = *it_info;

    uint64_t cost_one_vote = 0;
    if (info.count_votes > 0)
        cost_one_vote = double_to_i64(double_div(base_account.balance.amount, info.count_votes));

    vote_table table_vote(_creator, _creator);
    for (auto it = table_vote.begin(); it != table_vote.end(); ++it) {
        auto vote = *it;
        auto count_vote_in_post = generator::count_votes_in_post_reccord(vote);

        table_vote.modify( it, 0, [&]( auto& item ) {
            item.cost = asset((count_vote_in_post * cost_one_vote), string_to_symbol(4, "VIM"));
        });
    }
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

    update_info(info::up_vote);
    calculation_cost_posts();
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

    update_info(info::down_vote);
    calculation_cost_posts();
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

    update_info(info::create_post);
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
            item.released_tokens = 1000000000;
        };

        table_info.emplace( _creator, lambda_info );
    }
}
