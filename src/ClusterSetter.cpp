//
// Created by fagod on 05/08/2023.
//

#include "ClusterSetter.h"

void ClusterSetter::setup_cluster(dpp::cluster &bot, DBAdapter &db_adapter) {
  set_commands(bot, db_adapter);
  set_events(bot, db_adapter);
  set_timer(bot, db_adapter);
}

void ClusterSetter::set_commands(dpp::cluster &bot, DBAdapter &db_adapter) {
  bot.on_ready([&bot](const dpp::ready_t &event) {
    if (dpp::run_once<struct register_bot_commands>()) {
      register_text_channel_command(bot, "add_white_list");
      register_text_channel_command(bot, "delete_white_list");
    }
  });

  bot.on_slashcommand([&db_adapter](const dpp::slashcommand_t &event) {
    add_white_list(db_adapter, event);
    delete_white_list(db_adapter, event);
  });
}

void ClusterSetter::set_events(dpp::cluster &bot, DBAdapter &db_adapter) {
  bot.on_voice_state_update(
      [&db_adapter, &bot](const dpp::voice_state_update_t &event) {
        const auto user_id = static_cast<uint64_t>(event.state.user_id);
        const auto guild_id = static_cast<uint64_t>(event.state.guild_id);
        auto user = bot.user_get_sync(user_id);
        if (user.is_bot()) {
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
    auto msg = event.msg.content;
    if (!event.msg.stickers.empty()) {
      msg += " %STICKER%";
    }
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
}

void ClusterSetter::set_timer(dpp::cluster &bot, DBAdapter &db_adapter) {
  bot.start_timer(
      [&db_adapter, &bot](dpp::timer timer) {
        db_adapter.flush_time_count();
        auto *temp = db_adapter.calculate_user_points();
        for (const auto &[guild, user_and_points] : *temp) {
          //          string guild_name = dpp::find_guild(guild)->name;
          uint32_t max_points = 0;
          for (const auto &[__, points] : user_and_points) {
            uint32_t cur_points = points.first + points.second;
            max_points = cur_points > max_points ? cur_points : max_points;
          }

          for (const auto &[user, points] : user_and_points) {
            auto user_obj = bot.user_get_sync(user);
            if (points.first+points.second > max_points * 0.75) {
              // give role
              continue;
            }
            if (points.first+points.second > max_points * 0.5) {
              // give role
              continue;
            }
            if (points.first+points.second > max_points * 0.25) {
              // give role
              continue;
            }
            // give role

//              string user_name = user_obj.format_username();
//              spdlog::debug("On server {0} {1} has {2} points",
//              guild_name,
//                            user_name, points);
          }

          using pair_type = decltype(user_and_points)::value_type;
          auto max_messages = std::max_element
              (
                  std::begin(user_and_points), std::end(user_and_points),
                  [] (const pair_type & p1, const pair_type & p2) {
                    return p1.second.first < p2.second.first;
                  }
              );
          // give role

          auto max_voice = std::max_element
              (
                  std::begin(user_and_points), std::end(user_and_points),
                  [] (const pair_type & p1, const pair_type & p2) {
                    return p1.second.second < p2.second.second;
                  }
              );
          // give role
        }
      },
      300, [](dpp::timer timer) { spdlog::debug("timer stoped"); });
}

void ClusterSetter::add_white_list(DBAdapter &db_adapter,
                                   const dpp::slashcommand_t &event) {
  if (event.command.get_command_name() == "add_white_list") {
    uint64_t guild_id = event.command.guild_id;
    uint64_t channel_id =
        std::get<dpp::snowflake>(event.get_parameter("text_channel"));
    db_adapter.add_to_white_list(guild_id, channel_id);
    event.reply(dpp::message("channel added to white list")
                    .set_flags(dpp::m_ephemeral));
  }
}

void ClusterSetter::delete_white_list(DBAdapter &db_adapter,
                                      const dpp::slashcommand_t &event) {
  if (event.command.get_command_name() == "delete_white_list") {
    uint64_t guild_id = event.command.guild_id;
    uint64_t channel_id =
        std::get<dpp::snowflake>(event.get_parameter("text_channel"));
    if (!db_adapter.in_whitelist(guild_id, channel_id)) {
      event.reply(dpp::message("ERROR: channel not in white list")
                      .set_flags(dpp::m_ephemeral));
      return;
    }
    db_adapter.delete_from_white_list(guild_id, channel_id);
    event.reply(dpp::message("channel deleted from white list")
                    .set_flags(dpp::m_ephemeral));
  }
}

void ClusterSetter::register_text_channel_command(dpp::cluster &bot,
                                                  const string &command_name) {
  dpp::slashcommand slashcommand(command_name, "Choose text channel",
                                 bot.me.id);
  slashcommand.add_option(dpp::command_option(dpp::co_channel, "text_channel",
                                              "Choose an channel", true)
                              .add_channel_type(dpp::CHANNEL_TEXT));
  bot.global_command_create(slashcommand);
}
