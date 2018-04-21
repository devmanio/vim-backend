/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#include "eosio/hackathon_plugin/hackathon_plugin.hpp"

#include <eosio/chain/wast_to_wasm.hpp>
#include <eosio/chain/contracts/abi_serializer.hpp>

#include <eosio/wallet_plugin/wallet_plugin.hpp>
#include <eosio/wallet_plugin/wallet_manager.hpp>

#include <eosio/utilities/key_conversion.hpp>

#include <fc/variant.hpp>
#include <fc/io/json.hpp>
#include <fc/exception/exception.hpp>
#include <fc/reflect/variant.hpp>

#include <boost/algorithm/clamp.hpp>
#include <boost/asio/high_resolution_timer.hpp>

#include <Inline/BasicTypes.h>
#include <IR/Module.h>
#include <IR/Validate.h>
#include <WAST/WAST.h>
#include <WASM/WASM.h>
#include <Runtime/Runtime.h>

#include <hackathon/hackathon.wast.hpp>
#include <hackathon/hackathon.abi.hpp>


namespace eosio {

static appbase::abstract_plugin& _hackathon_plugin = app().register_plugin<hackathon_plugin>();

using namespace eosio::chain;

#define CALL(api_name, api_handle, call_name, INVOKE, http_response_code) \
{ \
    std::string("/v1/" #api_name "/" #call_name), \
    [this](string, string body, url_response_callback cb) mutable { \
        try { \
            if (body.empty()) body = "{}"; \
                INVOKE \
                cb(http_response_code, fc::json::to_string(result)); \
        } catch (fc::eof_exception& e) { \
            error_results results{400, "Bad Request", e}; \
            cb(400, fc::json::to_string(results)); \
            elog("Unable to parse arguments: ${args}", ("args", body)); \
        } catch (fc::exception& e) { \
            error_results results{500, "Internal Service Error", e}; \
            cb(500, fc::json::to_string(results)); \
            elog("Exception encountered while processing ${call}: ${e}", ("call", #api_name "." #call_name)("e", e)); \
        } \
    } \
}

#define INVOKE_V_R_R_R_R(api_handle, call_name, in_param0, in_param1, in_param2, in_param3) \
    const auto& vs = fc::json::json::from_string(body).as<fc::variants>(); \
    api_handle->call_name(vs.at(0).as<in_param0>(), vs.at(1).as<in_param1>(), vs.at(2).as<in_param2>(), vs.at(3).as<in_param3>()); \
    eosio::responce_structures::hackathon_plugin_empty result;

#define INVOKE_V_R_R_R(api_handle, call_name, in_param0, in_param1, in_param2) \
    const auto& vs = fc::json::json::from_string(body).as<fc::variants>(); \
    api_handle->call_name(vs.at(0).as<in_param0>(), vs.at(1).as<in_param1>(), vs.at(2).as<in_param2>()); \
    eosio::responce_structures::hackathon_plugin_empty result;

#define INVOKE_V_R_R(api_handle, call_name, in_param0, in_param1) \
    const auto& vs = fc::json::json::from_string(body).as<fc::variants>(); \
    api_handle->call_name(vs.at(0).as<in_param0>(), vs.at(1).as<in_param1>()); \
    eosio::responce_structures::hackathon_plugin_empty result;

#define INVOKE_V_R(api_handle, call_name, in_param0) \
    const auto& vs = fc::json::json::from_string(body).as<fc::variants>(); \
    api_handle->call_name(vs.at(0).as<in_param0>()); \
    eosio::responce_structures::hackathon_plugin_empty result;

#define INVOKE_V_V(api_handle, call_name) \
    api_handle->call_name(); \
    eosio::responce_structures::hackathon_plugin_empty result;

#define INVOKE_R_R(api_handle, call_name, in_param0) \
    const auto& vs = fc::json::json::from_string(body).as<fc::variants>(); \
    auto result = api_handle->call_name(vs.at(0).as<in_param0>());


hackathon_plugin::hackathon_plugin(){}
hackathon_plugin::~hackathon_plugin(){}

void hackathon_plugin::set_program_options(options_description&, options_description& cfg) {
    cfg.add_options()
            ("option-name", bpo::value<string>()->default_value("default value"),
             "Option Description")
            ;
}

void hackathon_plugin::plugin_initialize(const variables_map& options) {
    if(options.count("option-name")) {
        // Handle the option
    }
}

void hackathon_plugin::plugin_startup() {
    app().get_plugin<http_plugin>().add_api(
    {
        CALL(hackathon_plgn, _hackathon_plugin_impl, create_account, INVOKE_R_R(_hackathon_plugin_impl, create_account, std::string), 201),
        CALL(hackathon_plgn, _hackathon_plugin_impl, init_plugin, INVOKE_V_V(_hackathon_plugin_impl, init_plugin), 200),
        CALL(hackathon_plgn, _hackathon_plugin_impl, start_generation, INVOKE_V_R_R_R_R(_hackathon_plugin_impl,
             start_generation, uint64_t, uint64_t, uint64_t, uint64_t), 200),
        CALL(hackathon_plgn, _hackathon_plugin_impl, create_post, INVOKE_V_R_R_R_R(_hackathon_plugin_impl, create_post, std::string, uint64_t, std::string, std::string), 200),
        CALL(hackathon_plgn, _hackathon_plugin_impl, stop_generation, INVOKE_V_V(_hackathon_plugin_impl, stop_generation), 200),
        CALL(hackathon_plgn, _hackathon_plugin_impl, transfer, INVOKE_V_R_R_R(_hackathon_plugin_impl, transfer, std::string, std::string, std::string), 200),
        CALL(hackathon_plgn, _hackathon_plugin_impl, fund_account, INVOKE_V_R_R(_hackathon_plugin_impl, fund_account, std::string, std::string), 200),
        CALL(hackathon_plgn, _steepshot_plugin_impl, upvote, INVOKE_V_R_R(_hackathon_plugin_impl, upvote, uint64_t, std::string), 200),
        CALL(hackathon_plgn, _steepshot_plugin_impl, downvote, INVOKE_V_R_R(_hackathon_plugin_impl, downvote, uint64_t, std::string), 200)
    });

    _hackathon_plugin_impl.reset(new hackathon_plugin_impl);
}

void hackathon_plugin::plugin_shutdown() {
    try {
        _hackathon_plugin_impl->stop_generation();
    }
    catch(fc::exception e) {
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////
//                                class hackathon_plugin_impl                                 //
////////////////////////////////////////////////////////////////////////////////////////////////

hackathon_plugin_impl::hackathon_plugin_impl() :
    _hackathon("hackathon"),
    _hackathon_abi_def(fc::json::from_string(hackathon_abi).as<contracts::abi_def>()),
    _hackathon_serializer(_hackathon_abi_def),
    _hackathon_priv_key(fc::crypto::private_key::regenerate(fc::sha256(std::string(64, 'c')))),
    _chain_controller(app().get_plugin<chain_plugin>().chain()),
    _emission_timer(app().get_io_service()),
    _hash_counter(0)
{}

hackathon_plugin_impl::~hackathon_plugin_impl(){}

void hackathon_plugin_impl::emission_timer(const uint64_t &period_emission, const uint64_t &batch_size_emission) {
    auto start_time = boost::asio::high_resolution_timer::clock_type::now();
    _emission_timer.expires_at(start_time + std::chrono::milliseconds(period_emission));

    auto lambda = [=](const boost::system::error_code& code_error) {
        if(code_error) {
            elog("Error timer emission");
            return;
        }

        try {
            send_transaction(batch_size_emission);
        }
        catch(fc::exception e) {
            elog("pushing transaction failed: ${e}", ("e", e.to_detail_string()));
            stop_generation();
            return;
        }

        emission_timer(period_emission, batch_size_emission);
    };

    _emission_timer.async_wait(lambda);
}

void hackathon_plugin_impl::fund_account(const std::string &account, const std::string &balance) {
    action trx_action;
    trx_action.account = N(hackathon);
    trx_action.name = N(fundaccount);
    trx_action.authorization = vector<permission_level>{{_hackathon, config::active_name}};
    trx_action.data = _hackathon_serializer.variant_to_binary(
                "accbalance",
                fc::json::from_string(
                    fc::format_string(
                        "{\"account\":\"${account}\",\"balance\":\"${balance}\"}",
                        fc::mutable_variant_object()("account", account)("balance", balance))));

    signed_transaction trx;
    trx.actions.push_back(trx_action);
    trx.set_reference_block(_chain_controller.head_block_id());
    trx.expiration = _chain_controller.head_block_time() + fc::seconds(30);
    trx.sign(_hackathon_priv_key, _chain_controller.get_chain_id());
    _chain_controller.push_transaction(packed_transaction(trx));
}

void hackathon_plugin_impl::send_transaction(const uint64_t &m_batch) {
    for(unsigned int i = 0; i < m_batch; ++i) {
        action trx_action;
        trx_action.account = N(hackathon);
        trx_action.name = N(emission);
        trx_action.authorization = vector<permission_level>{{_hackathon, config::active_name}};
        trx_action.data = _hackathon_serializer.variant_to_binary(
                    "emission",
                    fc::json::from_string(
                        fc::format_string(
                            "{\"amount\":\"5.000 VIM\",\"hash\":\"${hash}\"}",
                            fc::mutable_variant_object()("hash", _hash_counter == 1000 ? _hash_counter = 0 : ++_hash_counter))));

        signed_transaction trx;
        trx.actions.push_back(trx_action);
        trx.set_reference_block(_chain_controller.head_block_id());
        trx.expiration = _chain_controller.head_block_time() + fc::seconds(30);
        trx.sign(_hackathon_priv_key, _chain_controller.get_chain_id());
        _chain_controller.push_transaction(packed_transaction(trx));
    }
}

void hackathon_plugin_impl::transfer(const std::string& from, const std::string& to, const std::string &amount) {
    wallet_manager& wallet_mgr = app().get_plugin<wallet_plugin>().get_wallet_manager();

    action trx_action;
    trx_action.account = N(hackathon);
    trx_action.name = N(transfer);
    trx_action.authorization = vector<permission_level>{ {name(from), config::active_name} };
    trx_action.data = _hackathon_serializer.variant_to_binary(
                "transfer",
                fc::json::from_string(
                    fc::format_string(
                        "{\"from\":\"${from}\",\"to\":\"${to}\",\"amount\":\"${amount}\"}",
                        fc::mutable_variant_object()("from", from)("to", to)("amount", amount))));

    signed_transaction trx;
    trx.actions.push_back(trx_action);
    trx.set_reference_block(_chain_controller.head_block_id());
    trx.expiration = _chain_controller.head_block_time() + fc::seconds(30);

    const auto &list_pub_keys = _chain_controller.get_required_keys( trx, wallet_mgr.get_public_keys() );
    for (auto pub_key : list_pub_keys)
        trx.sign( wallet_mgr.list_keys().at(pub_key), _chain_controller.get_chain_id() );

    _chain_controller.push_transaction(packed_transaction(trx));
}

void hackathon_plugin_impl::upvote(const uint64_t& uuid_post, const std::string& account) {
    action trx_action;
    trx_action.account = N(hackathon);
    trx_action.name = N(upvote);
    trx_action.authorization = vector<permission_level>{{_hackathon, config::active_name}};
    trx_action.data = _hackathon_serializer.variant_to_binary(
                "vote",
                fc::json::from_string(
                    fc::format_string(
                        "{\"uuid_post\":\"${uuid_post}\","
                        "\"account\":\"${account}\"}",
                        fc::mutable_variant_object()
                        ("uuid_post", uuid_post)
                        ("account", account))));

    signed_transaction trx;
    trx.actions.push_back(trx_action);
    trx.set_reference_block(_chain_controller.head_block_id());
    trx.expiration = _chain_controller.head_block_time() + fc::seconds(30);
    trx.sign(_hackathon_priv_key, _chain_controller.get_chain_id());
    _chain_controller.push_transaction(packed_transaction(trx));
}

void hackathon_plugin_impl::downvote(const uint64_t& uuid_post, const std::string& account) {
    action trx_action;
    trx_action.account = N(hackathon);
    trx_action.name = N(downvote);
    trx_action.authorization = vector<permission_level>{{_hackathon, config::active_name}};
    trx_action.data = _hackathon_serializer.variant_to_binary(
                "vote",
                fc::json::from_string(
                    fc::format_string(
                        "{\"uuid_post\":\"${uuid_post}\","
                        "\"account\":\"${account}\"}",
                        fc::mutable_variant_object()
                        ("uuid_post", uuid_post)
                        ("account", account))));

    signed_transaction trx;
    trx.actions.push_back(trx_action);
    trx.set_reference_block(_chain_controller.head_block_id());
    trx.expiration = _chain_controller.head_block_time() + fc::seconds(30);
    trx.sign(_hackathon_priv_key, _chain_controller.get_chain_id());
    _chain_controller.push_transaction(packed_transaction(trx));
}

void hackathon_plugin_impl::create_post(
        const std::string& account,
        const uint64_t& uuid_post,
        const std::string& url,
        const std::string& hash) {
    action trx_action;
    trx_action.account = N(hackathon);
    trx_action.name = N(createpost);
    trx_action.authorization = vector<permission_level>{{_hackathon, config::active_name}};
    trx_action.data = _hackathon_serializer.variant_to_binary(
                "post",
                fc::json::from_string(
                    fc::format_string(
                        "{\"account\":\"${account}\","
                        "\"uuid_post\":\"${uuid_post}\","
                        "\"url\":\"${url}\","
                        "\"hash\":\"${hash}\"}",
                        fc::mutable_variant_object()
                        ("account", account)
                        ("uuid_post", uuid_post)
                        ("url", url)
                        ("hash", hash))));

    signed_transaction trx;
    trx.actions.push_back(trx_action);
    trx.set_reference_block(_chain_controller.head_block_id());
    trx.expiration = _chain_controller.head_block_time() + fc::seconds(30);
    trx.sign(_hackathon_priv_key, _chain_controller.get_chain_id());
    _chain_controller.push_transaction(packed_transaction(trx));
}

void hackathon_plugin_impl::stop_generation() {
    if(!_is_running) {
        elog("Plugin not running");
        throw fc::exception(fc::invalid_operation_exception_code);
    }

    _emission_timer.cancel();

    _is_running = false;
    ilog("Stopping transaction generation");
}

eosio::responce_structures::resp_create_account hackathon_plugin_impl::create_account(const string &m_account_name)
{
    wallet_manager& wallet_mgr = app().get_plugin<wallet_plugin>().get_wallet_manager();

    auto password_account = wallet_mgr.create(m_account_name);

    auto owner_priv_key = private_key_type::generate();
    wallet_mgr.import_key(m_account_name, string(owner_priv_key));

    auto active_priv_key = private_key_type::generate();
    wallet_mgr.import_key(m_account_name, string(active_priv_key));

    {
        name producer_account("eosio");
        fc::crypto::private_key producer_priv_key(std::string("5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3"));

        signed_transaction trx;
        trx.expiration = _chain_controller.head_block_time() + fc::seconds(30);
        trx.set_reference_block(_chain_controller.head_block_id());

        auto owner_auth    = eosio::chain::authority{1, {{owner_priv_key.get_public_key(), 1}}, {}};
        auto active_auth   = eosio::chain::authority{1, {{active_priv_key.get_public_key(), 1}}, {}};
        auto recovery_auth = eosio::chain::authority{1, {}, {{{producer_account, "active"}, 1}}};

        trx.actions.emplace_back(vector<chain::permission_level>{{producer_account,"active"}},
                                 contracts::newaccount{producer_account, name(m_account_name),
                                                       owner_auth, active_auth, recovery_auth});

        trx.sign(producer_priv_key, _chain_controller.get_chain_id());
        _chain_controller.push_transaction(packed_transaction(trx));

        {
            action trx_action;
            trx_action.account = N(hackathon);
            trx_action.name = N(addaccount);
            trx_action.authorization = vector<permission_level>{{_hackathon, config::active_name}};
            trx_action.data = _hackathon_serializer.variant_to_binary(
                        "accinfo",
                        fc::json::from_string(
                            fc::format_string(
                                "{\"account\":\"${m_account_name}\"}",
                                fc::mutable_variant_object()
                                ("m_account_name", m_account_name))));

            signed_transaction trx;
            trx.actions.push_back(trx_action);
            trx.set_reference_block(_chain_controller.head_block_id());
            trx.expiration = _chain_controller.head_block_time() + fc::seconds(30);
            trx.sign(_hackathon_priv_key, _chain_controller.get_chain_id());
            _chain_controller.push_transaction(packed_transaction(trx));
        }

        ilog("'${name}' account was created successfully.", ("name", m_account_name));
    }


    wallet_mgr.lock(m_account_name);

    eosio::responce_structures::resp_create_account responce;
    responce.login = m_account_name;
    responce.pass = password_account;

    return responce;
}

void hackathon_plugin_impl::init_plugin()
{
    if (_is_init_plugin) {
        ilog("Plugin is already initialized!");
        return;
    }

    name producer_account("eosio");
    fc::crypto::private_key producer_priv_key(std::string("5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3"));
    _hackathon_pub_key = _hackathon_priv_key.get_public_key();

    create_base_account(producer_account, producer_priv_key);
    set_deploy_contract();

    _is_init_plugin = true;
}

void hackathon_plugin_impl::start_generation(const uint64_t &period_fund_account, const uint64_t &batch_size_fund_account,
                                             const uint64_t &period_emission, const uint64_t &batch_size_emission)
{
    if(_is_running) {
        elog("Generation is running");
        throw fc::exception(fc::invalid_operation_exception_code);
    }

    if(period_fund_account < 1 || period_fund_account > 2500) {
        elog("Period fund base account value is not in the range from 1 to 2500");
        throw fc::exception(fc::invalid_operation_exception_code);
    }

    if(period_emission < 1 || period_emission > 2500) {
        elog("Period emission value is not in the range from 1 to 2500");
        throw fc::exception(fc::invalid_operation_exception_code);
    }

    if(batch_size_fund_account < 1 || batch_size_fund_account > 250) {
        elog("Batch fund base account value is not in the range from 1 to 250");
        throw fc::exception(fc::invalid_operation_exception_code);
    }

    if(batch_size_emission < 1 || batch_size_emission > 250) {
        elog("Batch emission value is not in the range from 1 to 250");
        throw fc::exception(fc::invalid_operation_exception_code);
    }

    if(batch_size_fund_account & 1)
        throw fc::exception(fc::invalid_operation_exception_code);

    if(batch_size_emission & 1)
        throw fc::exception(fc::invalid_operation_exception_code);

    emission_timer(period_emission, batch_size_emission);
    ilog("Started emission in hackathon plugin; performing ${p} transactions every ${m}ms", ("p", batch_size_emission)("m", period_emission));

    _is_running = true;
}

void hackathon_plugin_impl::create_base_account(const account_name &m_producer, const fc::crypto::private_key &m_producer_priv_key)
{
    signed_transaction trx;
    trx.expiration = _chain_controller.head_block_time() + fc::seconds(30);
    trx.set_reference_block(_chain_controller.head_block_id());
    auto owner_auth    = eosio::chain::authority{1, {{_hackathon_pub_key, 1}}, {}};
    auto active_auth   = eosio::chain::authority{1, {{_hackathon_pub_key, 1}}, {}};
    auto recovery_auth = eosio::chain::authority{1, {}, {{{m_producer, "active"}, 1}}};

    trx.actions.emplace_back(vector<chain::permission_level>{{m_producer,"active"}},
                             contracts::newaccount{m_producer, _hackathon, owner_auth, active_auth, recovery_auth});

    trx.sign(m_producer_priv_key, _chain_controller.get_chain_id());
    _chain_controller.push_transaction(packed_transaction(trx));
    ilog("'hackathon' account was created successfully.");
}

void hackathon_plugin_impl::set_deploy_contract()
{
    signed_transaction trx;
    trx.expiration = _chain_controller.head_block_time() + fc::seconds(30);
    vector<uint8_t> wasm = wast_to_wasm(std::string(hackathon_wast));

    contracts::setcode handler;
    handler.account = _hackathon;
    handler.code.assign(wasm.begin(), wasm.end());
    trx.actions.emplace_back(vector<chain::permission_level>{{_hackathon, "active"}}, handler);

    {
        contracts::setabi handler;
        handler.account = _hackathon;
        handler.abi = _hackathon_abi_def;
        trx.actions.emplace_back(vector<chain::permission_level>{{_hackathon,"active"}}, handler);
    }

    trx.set_reference_block(_chain_controller.head_block_id());
    trx.sign(_hackathon_priv_key, _chain_controller.get_chain_id());
    _chain_controller.push_transaction(packed_transaction(trx));

    {
        action trx_action;
        trx_action.account = N(hackathon);
        trx_action.name = N(init);
        trx_action.authorization = vector<permission_level>{{_hackathon, config::active_name}};
        trx_action.data = _hackathon_serializer.variant_to_binary(
                    "init",
                    fc::json::from_string(
                        fc::format_string(
                            "{\"autoruzation\":\"${autoruzation}\"}",
                            fc::mutable_variant_object()
                            ("autoruzation", 1))));

        signed_transaction trx;
        trx.actions.push_back(trx_action);
        trx.set_reference_block(_chain_controller.head_block_id());
        trx.expiration = _chain_controller.head_block_time() + fc::seconds(30);
        trx.sign(_hackathon_priv_key, _chain_controller.get_chain_id());
        _chain_controller.push_transaction(packed_transaction(trx));
    }
    ilog("'hackathon' contract was implemented successfully.");
}

}


#undef INVOKE_V_R_R_R_R
#undef INVOKE_V_R_R_R
#undef INVOKE_V_R_R
#undef INVOKE_V_R
#undef INVOKE_V_V
#undef INVOKE_R_R
#undef CALL
