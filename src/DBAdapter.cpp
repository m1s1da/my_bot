//
// Created by fagod on 22/07/2023.
//

#include "DBAdapter.h"
#include <chrono>

#include "MessageChecker.h"

using std::to_string;

DBAdapter::DBAdapter(const string &db_path)
    : db_(db_path, SQLite::OPEN_READWRITE) {
  spdlog::info("DB is ready");
  json config;
  std::ifstream configfile("../config.json");
  configfile >> config;

  try {
    SECOND_COST = config["SECOND_COST"];
  } catch (std::exception &e) {
    SECOND_COST = 1;
    spdlog::info("can't find SECOND_COST, SECOND_COST = {}", SECOND_COST);
  }

  try {
    WORD_COST = config["WORD_COST"];
  } catch (std::exception &e) {
    WORD_COST = SECOND_COST * 100;
    spdlog::info("can't find WORD_COST, WORD_COST = {}", WORD_COST);
  }

  try {
    ATTACHMENT_COST = config["ATTACHMENT_COST"];
  } catch (std::exception &e) {
    ATTACHMENT_COST = WORD_COST * 3;
    spdlog::info("can't find ATTACHMENT_COST, ATTACHMENT_COST = {}",
                 ATTACHMENT_COST);
  }

  try {
    TRACKED_PERIOD = config["TRACKED_PERIOD"];
  } catch (std::exception &e) {
    TRACKED_PERIOD = 2592000 * 3;
    spdlog::info("can't find TRACKED_PERIOD, TRACKED_PERIOD = {} months",
                 static_cast<double>(TRACKED_PERIOD) / 2592000);
  }

  cash_white_list();
  cash_roles();
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

void DBAdapter::delete_from_white_list(const uint64_t &guild_id,
                                       const uint64_t &channel_id) {
  const string query_str =
      "DELETE FROM white_lists WHERE guild_id = ? AND channel_id = ?";
  SQLite::Statement query(db_, query_str);
  query.bind(1, to_string(guild_id));
  query.bind(2, to_string(channel_id));
  try {
    query.exec();
    auto it_guild = white_list_.find(guild_id);
    if (it_guild == white_list_.end()) {
      return;
    }
    auto it_channel =
        std::find(it_guild->second.begin(), it_guild->second.end(), channel_id);
    if (it_channel == it_guild->second.end()) {
      return;
    }
    it_guild->second.erase(it_channel);
  } catch (std::exception &e) {
    spdlog::error("delete_from_white_list: {}", e.what());
  }
  spdlog::debug("delete_from_white_list");
}

const DBAdapter::u_points *DBAdapter::calculate_user_points() {
  user_points_.clear();
  string query_str =
      "SELECT user_id, guild_id, SUM(word_counter), SUM(attachment_counter) "
      "FROM user_messages WHERE timestamp > ? "
      "GROUP BY user_id, guild_id";
  SQLite::Statement query_msg(db_, query_str);
  query_msg.bind(1, get_time_now() - TRACKED_PERIOD);
  try {
    while (query_msg.executeStep()) {
      const uint64_t user_id = std::stoull(query_msg.getColumn(0));
      const uint64_t guild_id = std::stoull(query_msg.getColumn(1));
      const uint32_t word_counter = std::stoul(query_msg.getColumn(2));
      const uint32_t attachment_counter = std::stoul(query_msg.getColumn(3));
      calculate_message_points(user_id, guild_id, word_counter,
                               attachment_counter);
    }
  } catch (std::exception &e) {
    spdlog::error("calculate_user_points: {}", e.what());
  }

  query_str = "SELECT user_id, guild_id, SUM(time_counter) "
              "FROM user_voice WHERE timestamp > ? "
              "GROUP BY user_id, guild_id";
  SQLite::Statement query_voice(db_, query_str);
  query_voice.bind(1, get_time_now() - TRACKED_PERIOD);
  try {
    while (query_voice.executeStep()) {
      const uint64_t user_id = std::stoull(query_voice.getColumn(0));
      const uint64_t guild_id = std::stoull(query_voice.getColumn(1));
      const uint32_t time_counter = std::stoul(query_voice.getColumn(2));
      calculate_voice_points(user_id, guild_id, time_counter);
    }
  } catch (std::exception &e) {
    spdlog::error("calculate_user_points: {}", e.what());
  }
  return &user_points_;
}

void DBAdapter::calculate_message_points(const uint64_t &user_id,
                                         const uint64_t &guild_id,
                                         const uint32_t &word_counter,
                                         const uint32_t &attachment_counter) {
  auto &guild = user_points_[guild_id];
  auto points = word_counter * WORD_COST + attachment_counter * ATTACHMENT_COST;
  guild[user_id].first += points;
}

void DBAdapter::calculate_voice_points(const uint64_t &user_id,
                                       const uint64_t &guild_id,
                                       const uint32_t &time_counter) {
  auto &guild = user_points_[guild_id];
  auto points = time_counter * SECOND_COST;
  guild[user_id].second += points;
}

void DBAdapter::add_role(const uint64_t &guild_id, const uint64_t &role_id,
                         const int64_t &percent) {
  const string query_str = "INSERT INTO GUILD_ROLES VALUES (?,?,?)";
  SQLite::Statement query(db_, query_str);
  query.bind(1, to_string(guild_id));
  query.bind(2, to_string(role_id));
  query.bind(3, percent);
  try {
    query.exec();
    roles_[guild_id].push_back({role_id, percent});
  } catch (std::exception &e) {
    spdlog::error("add_role: {}", e.what());
  }
  spdlog::debug("add_role");
}

void DBAdapter::delete_role(const uint64_t &guild_id, const uint64_t &role_id) {
  const string query_str =
      "DELETE FROM guild_roles WHERE guild_id = ? AND role_id = ?";
  SQLite::Statement query(db_, query_str);
  query.bind(1, to_string(guild_id));
  query.bind(2, to_string(role_id));
  try {
    query.exec();
    auto it_guild = roles_.find(guild_id);
    if (it_guild == roles_.end()) {
      return;
    }
    auto it_role = std::find_if(
        it_guild->second.begin(), it_guild->second.end(),
        [&](const pair<uint64_t, int64_t> &x) { return x.first == role_id; });
    if (it_role == it_guild->second.end()) {
      return;
    }
    it_guild->second.erase(it_role);
  } catch (std::exception &e) {
    spdlog::error("delete_role: {}", e.what());
  }
  spdlog::debug("delete_role");
}

void DBAdapter::cash_roles() {
  const string query_str = "SELECT * FROM guild_roles;";
  SQLite::Statement query(db_, query_str);
  try {
    while (query.executeStep()) {
      const uint64_t guild_id = std::stoull(query.getColumn(0));
      const uint64_t role_id = std::stoull(query.getColumn(1));
      const int64_t percent = std::stoi(query.getColumn(2));
      roles_[guild_id].push_back({role_id, percent});
    }
  } catch (std::exception &e) {
    spdlog::error("cash_roles: {}", e.what());
  }
  spdlog::debug("cash_roles");
}
const map<uint64_t, vector<pair<uint64_t, int64_t>>> &
DBAdapter::getRoles() const {
  return roles_;
}
