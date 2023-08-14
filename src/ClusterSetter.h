//
// Created by fagod on 05/08/2023.
//

#ifndef DISCORD_BOT_CLUSTERSETTER_H
#define DISCORD_BOT_CLUSTERSETTER_H
#include "DBAdapter.h"
#include <tavernbot/tavernbot.h>

class ClusterSetter {
public:
  static void setup_cluster(dpp::cluster &bot, DBAdapter &db_adapter);

private:
  static void set_commands(dpp::cluster &bot, DBAdapter &db_adapter);
  static void set_events(dpp::cluster &bot, DBAdapter &db_adapter);
  static void set_timer(dpp::cluster &bot, DBAdapter &db_adapter);

  static void update_roles(dpp::cluster &bot, DBAdapter &db_adapter,
                           const uint64_t &guild_id);

  /* Slashcommands */
  static dpp::slashcommand
  register_text_channel_command(dpp::cluster &bot,
                                            const string &command_name);
  static dpp::slashcommand register_add_role_command(dpp::cluster &bot);
  static dpp::slashcommand
  register_mentionable_command(dpp::cluster &bot,
                                           const string &command_name);
  static void add_white_list(DBAdapter &db_adapter,
                             const dpp::slashcommand_t &event);
  static void delete_white_list(DBAdapter &db_adapter,
                                const dpp::slashcommand_t &event);
  static void add_role(dpp::cluster &bot, DBAdapter &db_adapter,
                       const dpp::slashcommand_t &event);
  static void delete_role(dpp::cluster &bot, DBAdapter &db_adapter,
                          const dpp::slashcommand_t &event);

  /* Events */
  static void event_on_voice_state_update(dpp::cluster &bot,
                                          DBAdapter &db_adapter);
  static void event_on_guild_create(dpp::cluster &bot, DBAdapter &db_adapter);
  static void event_on_message_create(dpp::cluster &bot, DBAdapter &db_adapter);
};

#endif // DISCORD_BOT_CLUSTERSETTER_H
