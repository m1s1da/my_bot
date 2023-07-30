//
// Created by fagod on 22/07/2023.
//

#include "DBAdapter.h"
#include "dotenv.h"
#include <chrono>

#include "MessageChecker.h"

using std::to_string;

DBAdapter::DBAdapter(const string &db_path)
    : db_(db_path, SQLite::OPEN_READWRITE) {
  spdlog::info("DB is ready");
  cash_white_list();
}

uint32_t DBAdapter::get_time_now() {
  const auto ptr = std::chrono::system_clock::now();
  return std::chrono::duration_cast<std::chrono::seconds>(
             ptr.time_since_epoch())
      .count();
}

void DBAdapter::start_time_count(const uint64_t &user_id,
                                 const uint64_t &guild_id) {
  user_connected_timestamp_map_[{user_id, guild_id}] = get_time_now();
  spdlog::debug("start_time_count {0}, {1}:  time: {2}", user_id, guild_id,
                get_time_now());
}

void DBAdapter::write_time_overall(const uint64_t &user_id,
                                   const uint64_t &guild_id,
                                   const uint32_t &session_time_length) {
  const string query_str = "INSERT INTO user_voice "
                           "VALUES (?,?,?,?)";
  SQLite::Statement query(db_, query_str);
  query.bind(1, to_string(user_id));
  query.bind(2, to_string(guild_id));
  query.bind(3, session_time_length);
  query.bind(4, get_time_now());

  try {
    query.exec();
  } catch (std::exception &e) {
    spdlog::error("write_time_overall: {}", e.what());
  }
  spdlog::debug("write_time_overall {0}, {1}", user_id, guild_id);
}

void DBAdapter::stop_time_count(const uint64_t &user_id,
                                const uint64_t &guild_id) {
  auto it = user_connected_timestamp_map_.find({user_id, guild_id});
  if (it == user_connected_timestamp_map_.end()) {
    spdlog::error("ERROR in stop_time_count {0}, {1}: not found!", user_id,
                  guild_id);
    return;
  }
  const uint32_t session_time_length = get_time_now() - it->second;
  user_connected_timestamp_map_.erase(it);
  write_time_overall(user_id, guild_id, session_time_length);
  spdlog::debug("stop_time_count {0}, {1}:  session_time_length: {2}", user_id,
                guild_id, session_time_length);
}

void DBAdapter::write_message_info(const uint64_t &user_id,
                                   const uint64_t &guild_id, const string &msg,
                                   bool has_attachments) {
  const uint32_t word_counter = MessageChecker::getWordCounter(msg);
  const string query_str = "INSERT INTO user_messages "
                           "VALUES (?,?,?,?,?)";
  SQLite::Statement query(db_, query_str);
  query.bind(1, to_string(user_id));
  query.bind(2, to_string(guild_id));
  query.bind(3, word_counter);
  query.bind(4, static_cast<int>(has_attachments));
  query.bind(5, get_time_now());
  try {
    query.exec();
  } catch (std::exception &e) {
    spdlog::error("write_message_info: {}", e.what());
  }

  spdlog::debug("message received from: {0}, {1} word_counter: {2} msg: {3}",
                user_id, guild_id, word_counter, msg);
}

bool DBAdapter::in_connected(const uint64_t &user_id,
                             const uint64_t &guild_id) {
  return user_connected_timestamp_map_.contains({user_id, guild_id});
}

void DBAdapter::flush_time_count() {
  for (auto &[user_and_guild, timestamp] : user_connected_timestamp_map_) {
    const uint32_t session_time_length = get_time_now() - timestamp;
    timestamp = get_time_now();
    write_time_overall(user_and_guild.first, user_and_guild.second,
                       session_time_length);
  }
  spdlog::debug("flush_time_count completed");
}

void DBAdapter::add_to_white_list(const uint64_t &guild_id,
                                  const uint64_t &channel_id) {
  const string query_str = "INSERT INTO white_lists VALUES (?,?)";
  SQLite::Statement query(db_, query_str);
  query.bind(1, to_string(guild_id));
  query.bind(2, to_string(channel_id));
  try {
    query.exec();
    white_list_[guild_id].push_back(channel_id);
  } catch (std::exception &e) {
    spdlog::error("add_to_white_list: {}", e.what());
  }
  spdlog::debug("add_to_white_list");
}

void DBAdapter::cash_white_list() {
  const string query_str = "SELECT * FROM white_lists;";
  SQLite::Statement query(db_, query_str);
  try {
    while (query.executeStep()) {
      const uint64_t guild_id = std::stoull(query.getColumn(0));
      const uint64_t channel_id = std::stoull(query.getColumn(1));
      white_list_[guild_id].push_back(channel_id);
    }
  } catch (std::exception &e) {
    spdlog::error("cash_white_list: {}", e.what());
  }
  spdlog::debug("cash_white_list");
}

bool DBAdapter::in_whitelist(const uint64_t &guild_id,
                             const uint64_t &channel_id) {
  auto it_guild = white_list_.find(guild_id);
  // if guild is not in the whitelists -> whitelists not configured -> return
  // true
  if (it_guild == white_list_.end()) {
    return true;
  }
  auto it_channel =
      std::find(it_guild->second.begin(), it_guild->second.end(), channel_id);
  return it_channel != it_guild->second.end();
}
