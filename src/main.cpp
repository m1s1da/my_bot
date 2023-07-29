#include <dpp/dpp.h>
#include "dotenv.h"

#include "DBAdapter.h"
#include "spdlog/spdlog.h"

int main() {
#ifdef NDEBUG
    spdlog::set_level(spdlog::level::info);
#else
    spdlog::set_level(spdlog::level::debug);
#endif

    auto &dotenv = dotenv::env.load_dotenv("../.env", true);
    dpp::cluster bot(dotenv["BOT_TOKEN"], dpp::i_default_intents | dpp::i_message_content);

    DBAdapter db_adapter(dotenv["DB_PATH"]);
    bot.on_log(dpp::utility::cout_logger());

    bot.on_ready([&bot](const dpp::ready_t &event) {
        if (dpp::run_once<struct register_bot_commands>()) {
            bot.global_command_create(
                    dpp::slashcommand("ping", "Ping pong!", bot.me.id)
            );
        }
    });

    bot.on_slashcommand([](const dpp::slashcommand_t &event) {
        if (event.command.get_command_name() == "ping") {
            event.reply("Pong!");
        }
    });

    bot.on_voice_state_update([&db_adapter](const dpp::voice_state_update_t &event) {
        const auto user_id = static_cast<uint64_t>(event.state.user_id);
        const auto guild_id = static_cast<uint64_t>(event.state.guild_id);
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
        for (const auto &[user_id, voice_state]: voice_members) {
            auto guild_id = static_cast<uint64_t>(event.created->id);
            db_adapter.start_time_count(static_cast<uint64_t>(user_id), guild_id);
        }
    });

    bot.on_message_create([&db_adapter](const dpp::message_create_t &event) {
        const auto msg = event.msg.content;
        const auto user_id = static_cast<uint64_t>(event.msg.author.id);
        const auto guild_id = static_cast<uint64_t>(event.msg.guild_id);
        const bool has_attachments = !event.msg.attachments.empty();
        db_adapter.write_message_info(user_id, guild_id, msg, has_attachments);
    });

    bot.start_timer([&db_adapter](dpp::timer
                                  timer) {
        db_adapter.flush_time_count();
    }, 300, [](dpp::timer
               timer) { spdlog::debug("timer stoped"); });

    bot.start(dpp::st_wait);
}