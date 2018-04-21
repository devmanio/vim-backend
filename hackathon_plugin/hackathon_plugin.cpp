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

namespace eosio {

static appbase::abstract_plugin& _hackathon_plugin = app().register_plugin<hackathon_plugin>();

using namespace eosio::chain;

#define INVOKE_V_V(api_handle, call_name) \
    api_handle->call_name(); \
    eosio::responce_structures::hackathon_plugin_empty result;

#define INVOKE_R_R(api_handle, call_name, in_param0) \
    const auto& vs = fc::json::json::from_string(body).as<fc::variants>(); \
    auto result = api_handle->call_name(vs.at(0).as<in_param0>());

hackathon_plugin::hackathon_plugin():my(new hackathon_plugin_impl()){}
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
        CALL(hackathon_plgn, _hackathon_plugin_impl, init_plugin, INVOKE_V_V(_hackathon_plugin_impl, init_plugin), 200)
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
    _hash_counter(0)
{}

hackathon_plugin_impl::~hackathon_plugin_impl(){}

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
    ilog("'hackathon' contract was implemented successfully.");
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

}
