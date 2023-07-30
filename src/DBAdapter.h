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

//#define SECOND_COST 1
//#define WORD_COST SECOND_COST * 60 * 10
//#define ATTACHMENT_COST WORD_COST * 3

using std::map;
using std::pair;
using std::string;
using std::vector;

class DBAdapter {
public:
  DBAdapter(const string &db_path);

  using u_points = map<uint64_t, vector<pair<uint64_t, uint32_t>>>;

  void start_time_count(const uint64_t &user_id, const uint64_t &guild_id);

  void stop_time_count(const uint64_t &user_id, const uint64_t &guild_id);

  void flush_time_count();

  void write_message_info(const uint64_t &user_id, const uint64_t &guild_id,
                          const string &msg, bool has_attachments);

  bool in_connected(const uint64_t &user_id, const uint64_t &guild_id);

  void add_to_white_list(const uint64_t &guild_id, const uint64_t &channel_id);

  void delete_from_white_list(const uint64_t &guild_id,
                              const uint64_t &channel_id);

  bool in_whitelist(const uint64_t &guild_id, const uint64_t &channel_id);

  const u_points *calculate_user_points();

private:
  void cash_white_list();

  void calculate_message_points(const uint64_t &user_id,
                                const uint64_t &guild_id,
                                const uint32_t &word_counter,
                                const uint32_t &attachment_counter);

  void calculate_voice_points(const uint64_t &user_id, const uint64_t &guild_id,
                              const uint32_t &time_counter);

  void write_time_overall(const uint64_t &user_id, const uint64_t &guild_id,
                          const uint32_t &session_time_length);

  static uint32_t get_time_now();

private:
  SQLite::Database db_;

  map<pair<uint64_t, uint64_t>, uint64_t> user_connected_timestamp_map_;
  map<uint64_t, vector<uint64_t>> white_list_;
  u_points user_points_;

  uint32_t SECOND_COST;
  uint32_t WORD_COST;
  uint32_t ATTACHMENT_COST;
};

#endif // DISCORD_BOT_DBADAPTER_H
