#include "dotenv.h"
#include <dpp/dpp.h>

#include "DBAdapter.h"
#include "spdlog/spdlog.h"

int main() {
#ifdef NDEBUG
  spdlog::set_level(spdlog::level::info);
#else
  spdlog::set_level(spdlog::level::debug);
#endif

  auto &dotenv = dotenv::env.load_dotenv("../.env", true);
  dpp::cluster bot(dotenv["BOT_TOKEN"],
                   dpp::i_default_intents | dpp::i_message_content);

  DBAdapter db_adapter(dotenv["DB_PATH"]);
  bot.on_log(dpp::utility::cout_logger());

  bot.on_ready([&bot](const dpp::ready_t &event) {
    if (dpp::run_once<struct register_bot_commands>()) {
      dpp::slashcommand add_white_list("add_white_list", "Choose text channels",
                                   bot.me.id);
      add_white_list.add_option(dpp::command_option(dpp::co_channel, "text_channel",
                                                "Choose an channel", true)
                                .add_channel_type(dpp::CHANNEL_TEXT));
      bot.global_command_create(add_white_list);

      dpp::slashcommand delete_white_list("delete_white_list", "Choose text channels",
                                       bot.me.id);
      delete_white_list.add_option(dpp::command_option(dpp::co_channel, "text_channel",
                                                    "Choose an channel", true)
                                    .add_channel_type(dpp::CHANNEL_TEXT));
      bot.global_command_create(delete_white_list);
    }
  });

  bot.on_slashcommand([&db_adapter](const dpp::slashcommand_t &event) {
    if (event.command.get_command_name() == "add_white_list") {
      uint64_t guild_id = event.command.guild_id;
      uint64_t channel_id =
          std::get<dpp::snowflake>(event.get_parameter("text_channel"));
      db_adapter.add_to_white_list(guild_id, channel_id);
      event.reply("channel added to white list");
    }
    if (event.command.get_command_name() == "delete_white_list") {
      uint64_t guild_id = event.command.guild_id;
      uint64_t channel_id =
          std::get<dpp::snowflake>(event.get_parameter("text_channel"));
      if (!db_adapter.in_whitelist(guild_id, channel_id)) {
        event.reply("error: channel not in white list");
        return;
      }
      db_adapter.delete_from_white_list(guild_id, channel_id);
      event.reply("channel deleted from to white list");
    }
  });

  bot.on_voice_state_update(
      [&db_adapter](const dpp::voice_state_update_t &event) {
        const auto user_id = static_cast<uint64_t>(event.state.user_id);
        const auto guild_id = static_cast<uint64_t>(event.state.guild_id);
        if (dpp::find_user(user_id)->is_bot()) {
          return;
        }
        if (!event.state.channel_id.empty()) {
          // CONNECTED
          if (db_adapter.in_connected(user_id, guild_id)) {
            return;
          }
          db_adapter.start_time_count(user_id, guild_id);
        } else {
          // DISCONNECTED
          db_adapter.stop_time_count(user_id, guild_id);
        }
      });

  bot.on_guild_create([&db_adapter](const dpp::guild_create_t &event) {
    auto voice_members = event.created->voice_members;
    for (const auto &[user_id, voice_state] : voice_members) {
      auto guild_id = static_cast<uint64_t>(event.created->id);
      if (dpp::find_user(user_id)->is_bot()) {
        continue;
      }
      db_adapter.start_time_count(static_cast<uint64_t>(user_id), guild_id);
    }
  });

  bot.on_message_create([&db_adapter](const dpp::message_create_t &event) {
    const auto msg = event.msg.content;
    const auto user_id = static_cast<uint64_t>(event.msg.author.id);
    const auto guild_id = static_cast<uint64_t>(event.msg.guild_id);
    const auto channel_id = static_cast<uint64_t>(event.msg.channel_id);
    const bool has_attachments = !event.msg.attachments.empty();
    if (event.msg.author.is_bot()) {
      return;
    }
    if (db_adapter.in_whitelist(guild_id, channel_id)) {
      db_adapter.write_message_info(user_id, guild_id, msg, has_attachments);
    }
  });

  bot.start_timer(
      [&db_adapter](dpp::timer timer) { db_adapter.flush_time_count(); }, 300,
      [](dpp::timer timer) { spdlog::debug("timer stoped"); });

  bot.start(dpp::st_wait);
}