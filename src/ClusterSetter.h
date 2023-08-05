//
// Created by fagod on 05/08/2023.
//

#ifndef DISCORD_BOT_CLUSTERSETTER_H
#define DISCORD_BOT_CLUSTERSETTER_H
#include "DBAdapter.h"
#include <dpp/dpp.h>



class ClusterSetter {
public:
  static void setup_cluster(dpp::cluster &bot, DBAdapter &db_adapter);

private:
  static void set_commands(dpp::cluster &bot, DBAdapter &db_adapter);
  static void set_events(dpp::cluster &bot, DBAdapter &db_adapter);
  static void set_timer(dpp::cluster &bot, DBAdapter &db_adapter);

  static void register_text_channel_command(dpp::cluster &bot, const string &command_name);

  /* Slashcommands */
  static void add_white_list(DBAdapter &db_adapter, const dpp::slashcommand_t &event);
  static void delete_white_list(DBAdapter &db_adapter, const dpp::slashcommand_t &event);
};

#endif // DISCORD_BOT_CLUSTERSETTER_H
