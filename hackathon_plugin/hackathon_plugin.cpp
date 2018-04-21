/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#include <eosio/hackathon_plugin/hackathon_plugin.hpp>

namespace eosio {
   static appbase::abstract_plugin& _hackathon_plugin = app().register_plugin<hackathon_plugin>();

class hackathon_plugin_impl {
   public:
};

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
   // Make the magic happen
}

void hackathon_plugin::plugin_shutdown() {
   // OK, that's enough magic
}

}
