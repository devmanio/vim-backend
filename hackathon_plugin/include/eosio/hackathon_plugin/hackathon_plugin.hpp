/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#pragma once
#include <appbase/application.hpp>
#include <eosio/http_plugin/http_plugin.hpp>

#include <eosio/chain/chain_controller.hpp>
#include <eosio/chain_plugin/chain_plugin.hpp>

#include <eosio/chain/contracts/types.hpp>


namespace eosio {
    namespace responce_structures {
        struct hackathon_plugin_empty {};

        struct resp_create_account {
            std::string login;
            std::string pass;
        };
    }
}

namespace eosio {

using namespace appbase;
using namespace chain;

class hackathon_plugin_impl {
	public:
        hackathon_plugin_impl();
        virtual ~hackathon_plugin_impl();

        void init_plugin();
        void emission_timer(const uint64_t& period_emission, const uint64_t& batch_size_emission);
        void transfer(const std::string& from, const std::string& to, const std::string& amount);
        void fund_account(const std::string& account, const std::string& balance);
        void send_transaction(const uint64_t &m_batch);
        void start_generation(const uint64_t &period_fund_account, const uint64_t& batch_size_fund_account,
                              const uint64_t& period_emission, const uint64_t& batch_size_emission);
        void create_post(const std::string& account, const uint64_t& uuid_post, const std::string& url, const std::string& hash);
        void stop_generation();
        void upvote(const uint64_t& id_post, const std::string& account);
        void downvote(const uint64_t& id_post, const std::string& account);

        eosio::responce_structures::resp_create_account create_account(const std::string &m_account_name);

    private:
        void create_base_account(const account_name &m_producer, const fc::crypto::private_key &m_producer_priv_key);
        void set_deploy_contract();

    private:
        name _hackathon;
        contracts::abi_def _hackathon_abi_def;
        chain::contracts::abi_serializer _hackathon_serializer;
        fc::crypto::public_key _hackathon_pub_key;
        fc::crypto::private_key _hackathon_priv_key;
        chain_controller &_chain_controller;

        boost::asio::high_resolution_timer _emission_timer;

        bool _is_running = false;
        bool _is_init_plugin = false;

        int _hash_counter;
};

class hackathon_plugin : public appbase::plugin<hackathon_plugin> {
public:
   hackathon_plugin();
   virtual ~hackathon_plugin();
 
   APPBASE_PLUGIN_REQUIRES((http_plugin))
   virtual void set_program_options(options_description&, options_description& cfg) override;
 
   void plugin_initialize(const variables_map& options);
   void plugin_startup();
   void plugin_shutdown();

private:
   std::unique_ptr<class hackathon_plugin_impl> _hackathon_plugin_impl;
};

}

FC_REFLECT(eosio::responce_structures::hackathon_plugin_empty, );
FC_REFLECT(eosio::responce_structures::resp_create_account, (login)(pass));
