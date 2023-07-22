#include <dpp/dpp.h>

#include "dotenv.h"

#include "DBAdapter.h"

int main() {
    std::map<uint64_t, uint64_t> user_time_map;
    auto &dotenv = dotenv::env.load_dotenv("../.env", true);
    dpp::cluster bot(dotenv["BOT_TOKEN"], dpp::i_default_intents | dpp::i_message_content);
    DBAdapter db_adapter;
    bot.on_log(dpp::utility::cout_logger());

    bot.on_slashcommand([](const dpp::slashcommand_t &event) {
        if (event.command.get_command_name() == "ping") {
            event.reply("Pong!");
        }
    });

    bot.on_voice_state_update([&db_adapter](const dpp::voice_state_update_t &event) {
        if (!event.state.channel_id.empty()) {
            // CONNECTED
            db_adapter.start_time_count(static_cast<uint64_t>(event.state.user_id));
        } else {
            // DISCONNECTED
            db_adapter.stop_time_count(static_cast<uint64_t>(event.state.user_id));
        }
    });

    bot.on_ready([&bot](const dpp::ready_t &event) {
        if (dpp::run_once<struct register_bot_commands>()) {
            bot.global_command_create(
                    dpp::slashcommand("ping", "Ping pong!", bot.me.id)
            );
        }
    });

    bot.on_guild_create([& db_adapter](const dpp::guild_create_t &event) {
        auto voice_members = event.created->voice_members;
        for (const auto &[user_id, voice_state]: voice_members) {
            db_adapter.start_time_count(static_cast<uint64_t>(user_id));
        }
    });

    bot.on_message_create([&bot, &db_adapter](const dpp::message_create_t &event){
        std::string msg = event.msg.content;
        uint64_t user = event.msg.author.id;
        db_adapter.write_message_info(user, msg.size());
    });

    bot.start(dpp::st_wait);
}