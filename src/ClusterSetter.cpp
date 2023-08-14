//
// Created by fagod on 05/08/2023.
//

#include "ClusterSetter.h"

void ClusterSetter::setup_cluster(dpp::cluster &bot, DBAdapter &db_adapter) {
  bot.on_log(dpp::utility::cout_logger());
  set_commands(bot, db_adapter);
  set_events(bot, db_adapter);
  set_timer(bot, db_adapter);
}

void ClusterSetter::set_commands(dpp::cluster &bot, DBAdapter &db_adapter) {
  bot.on_ready([&bot](const dpp::ready_t &event) {
    if (dpp::run_once<struct register_bot_commands>()) {
      vector<dpp::slashcommand> slashcommands;
      slashcommands.emplace_back(
          register_text_channel_command(bot, "add_white_list"));
      slashcommands.emplace_back(
          register_text_channel_command(bot, "delete_white_list"));
      slashcommands.emplace_back(register_add_role_command(bot));
      slashcommands.emplace_back(
          register_mentionable_command(bot, "delete_role"));
      slashcommands.emplace_back("test", "test test!", bot.me.id);
      bot.global_bulk_command_create(slashcommands);
    }
  });

  bot.on_slashcommand([&bot, &db_adapter](const dpp::slashcommand_t &event) {
    add_white_list(db_adapter, event);
    delete_white_list(db_adapter, event);
    add_role(bot, db_adapter, event);
    delete_role(bot, db_adapter, event);
    if (event.command.get_command_name() == "test") {
      update_roles(bot, db_adapter, uint64_t{609047867935424520});
      event.reply(dpp::message("test").set_flags(dpp::m_ephemeral));
    }
  });
}

void ClusterSetter::set_events(dpp::cluster &bot, DBAdapter &db_adapter) {
  event_on_voice_state_update(bot, db_adapter);
  event_on_guild_create(bot, db_adapter);
  event_on_message_create(bot, db_adapter);
}

void ClusterSetter::set_timer(dpp::cluster &bot, DBAdapter &db_adapter) {
  bot.start_timer(
      [&db_adapter, &bot](dpp::timer timer) {
        db_adapter.flush_time_count();
        bot.current_user_get_guilds(
            [&](const dpp::confirmation_callback_t &callback) {
              auto guilds = std::get<dpp::guild_map>(callback.value);
              for (const auto &guild : guilds) {
                update_roles(bot, db_adapter,
                             static_cast<uint64_t>(guild.first));
              }
            });
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

void ClusterSetter::add_role(dpp::cluster &bot, DBAdapter &db_adapter,
                             const dpp::slashcommand_t &event) {
  if (event.command.get_command_name() == "add_role") {
    uint64_t guild_id = event.command.guild_id;
    const auto role_name = std::get<string>(event.get_parameter("role_name"));
    const auto percent = std::get<int64_t>(event.get_parameter("percent"));
    const auto is_best_in_text =
        std::get<bool>(event.get_parameter("is_best_in_text"));
    const auto is_best_in_voice =
        std::get<bool>(event.get_parameter("is_best_in_voice"));
    if (percent > 99 && !(is_best_in_voice || is_best_in_text)) {
      event.reply(
          dpp::message("ERROR: more than 99%").set_flags(dpp::m_ephemeral));
      return;
    }
    dpp::role new_role;
    new_role.set_name(role_name);
    new_role.set_guild_id(guild_id);
    std::thread t1([&]() {
      db_adapter.add_role(guild_id, bot.role_create_sync(new_role).id, percent,
                          is_best_in_text, is_best_in_voice);
      event.reply(dpp::message("role created").set_flags(dpp::m_ephemeral));
    });
    t1.join();
  }
}

void ClusterSetter::delete_role(dpp::cluster &bot, DBAdapter &db_adapter,
                                const dpp::slashcommand_t &event) {
  if (event.command.get_command_name() == "delete_role") {
    uint64_t guild_id = event.command.guild_id;
    const auto role_id = static_cast<uint64_t>(
        std::get<dpp::snowflake>(event.get_parameter("name")));

    std::thread t1([&]() {
      if (bot.roles_get_sync(guild_id).contains(role_id)) {
        bot.role_delete(guild_id, role_id);
        db_adapter.delete_role(guild_id, role_id);
        update_roles(bot, db_adapter, guild_id);
        event.reply(dpp::message("role deleted").set_flags(dpp::m_ephemeral));
      } else {
        event.reply(
            dpp::message("ERROR: not a role").set_flags(dpp::m_ephemeral));
      }
    });
    t1.join();
  }
}

dpp::slashcommand
ClusterSetter::register_text_channel_command(dpp::cluster &bot,
                                             const string &command_name) {
  dpp::slashcommand slashcommand(command_name, "Choose text channel",
                                 bot.me.id);
  slashcommand.add_option(dpp::command_option(dpp::co_channel, "text_channel",
                                              "Choose an channel", true)
                              .add_channel_type(dpp::CHANNEL_TEXT));
  return slashcommand;
}

void ClusterSetter::update_roles(dpp::cluster &bot, DBAdapter &db_adapter,
                                 const uint64_t &guild_id) {
  auto *user_and_points = db_adapter.calculate_user_points(guild_id);
  if (user_and_points->empty()) {
    return;
  }
  dpp::role_map *roles;
  // request
  std::thread t_roles(
      [&]() { roles = new dpp::role_map(bot.roles_get_sync(guild_id)); });
  t_roles.join();

  map<dpp::snowflake, dpp::members_container> roles_and_members;
  const auto &notable_roles = db_adapter.get_roles().find(guild_id)->second;
  for (auto &[role_id, role_obj] : *roles) {
    auto it =
        std::find_if(notable_roles.begin(), notable_roles.end(),
                     [&](const GuildRole &a) { return a.role_id == role_id; });

    if (it != notable_roles.end()) {
      // request
      roles_and_members[role_id] = role_obj.get_members();
    }
  }
  delete roles;
  /* max points finding */
  uint32_t max_points = 0;
  for (const auto &[_, points] : *user_and_points) {
    uint32_t cur_points = points.first + points.second;
    max_points = cur_points > max_points ? cur_points : max_points;
  }

  for (const auto &[user_id, points] : *user_and_points) {
    uint32_t cur_points = points.first + points.second;
    for (const auto &role : notable_roles) {
      if (role.is_best_in_voice || role.is_best_in_text) {
        continue;
      }
      if (cur_points > max_points * role.percent / 100) {
        auto &members = roles_and_members[role.role_id];
        auto it = members.find(user_id);
        if (it != members.end()) {
          // if we found user - role is correct
          members.erase(it);
          break;
        }
        // request
        bot.guild_member_add_role(guild_id, user_id, role.role_id);
        break;
      }
    }
  }

  for (auto &[role_id, members] : roles_and_members) {
    for (auto &member : members) {
      // request
      bot.guild_member_remove_role(guild_id, member.first, role_id);
    }
  }

  using pair_type =
      std::remove_reference_t<decltype(*user_and_points)>::value_type;
  auto get_role_max =
      [&](const std::function<bool(const GuildRole &)> &callback,
          const std::function<bool(const pair_type &, const pair_type &)>
              &callback2) {
        auto it =
            std::find_if(notable_roles.begin(), notable_roles.end(), callback);
        if (it != notable_roles.end()) {
          auto max_el = std::max_element(user_and_points->begin(),
                                         user_and_points->end(), callback2);
          // request
          bot.guild_member_add_role(guild_id, max_el->first, it->role_id);
        }
      };

  get_role_max([](const GuildRole &a) { return a.is_best_in_text; },
               [](const pair_type &p1, const pair_type &p2) {
                 return p1.second.first < p2.second.first;
               });

  get_role_max([](const GuildRole &a) { return a.is_best_in_voice; },
               [](const pair_type &p1, const pair_type &p2) {
                 return p1.second.second < p2.second.second;
               });
}

void ClusterSetter::event_on_voice_state_update(dpp::cluster &bot,
                                                DBAdapter &db_adapter) {
  bot.on_voice_state_update(
      [&db_adapter, &bot](const dpp::voice_state_update_t &event) {
        const auto user_id = static_cast<uint64_t>(event.state.user_id);
        const auto guild_id = static_cast<uint64_t>(event.state.guild_id);
        std::thread t1([&]() {
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
        t1.join();
      });
}

void ClusterSetter::event_on_guild_create(dpp::cluster &bot,
                                          DBAdapter &db_adapter) {
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
}

void ClusterSetter::event_on_message_create(dpp::cluster &bot,
                                            DBAdapter &db_adapter) {
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

dpp::slashcommand ClusterSetter::register_add_role_command(dpp::cluster &bot) {
  dpp::slashcommand slashcommand("add_role", "Add ranked role", bot.me.id);
  slashcommand
      .add_option(dpp::command_option(dpp::co_string, "role_name",
                                      "Type a role name", true))
      .add_option(dpp::command_option(dpp::co_integer, "percent",
                                      "Type a percent", true))
      .add_option(dpp::command_option(dpp::co_boolean, "is_best_in_text",
                                      "Is best in text?", true))
      .add_option(dpp::command_option(dpp::co_boolean, "is_best_in_voice",
                                      "Is best in voice?", true));
  return slashcommand;
}

dpp::slashcommand
ClusterSetter::register_mentionable_command(dpp::cluster &bot,
                                            const string &command_name) {
  dpp::slashcommand slashcommand(command_name, "Chose a variant", bot.me.id);
  slashcommand.add_option(dpp::command_option(
      dpp::co_mentionable, "mentionable", "Chose a variant", true));
  return slashcommand;
}
