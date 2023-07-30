//
// Created by fagod on 22/07/2023.
//

#ifndef DISCORD_BOT_DBADAPTER_H
#define DISCORD_BOT_DBADAPTER_H

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include <SQLiteCpp/SQLiteCpp.h>

using std::string;

class DBAdapter {
public:
  DBAdapter(const string &db_path);

  using ug_pair = std::pair<uint64_t, uint64_t>;

  void start_time_count(const uint64_t &user_id, const uint64_t &guild_id);

  void stop_time_count(const uint64_t &user_id, const uint64_t &guild_id);

  void flush_time_count();

  void write_message_info(const uint64_t &user_id, const uint64_t &guild_id,
                          const string &msg, bool has_attachments);

  bool in_connected(const uint64_t &user_id, const uint64_t &guild_id);

  void add_to_white_list(const uint64_t &guild_id, const uint64_t &channel_id);

  void cash_white_list();

  bool in_whitelist(const uint64_t &guild_id, const uint64_t &channel_id);

private:
  void write_time_overall(const uint64_t &user_id, const uint64_t &guild_id,
                          const uint32_t &session_time_length);

  static uint32_t get_time_now();

private:
  std::map<ug_pair, uint64_t> user_connected_timestamp_map_;
  std::map<uint64_t, std::vector<uint64_t>> white_list_;

  SQLite::Database db_;
  std::shared_ptr<spdlog::logger> err_logger_;
};

#endif // DISCORD_BOT_DBADAPTER_H
