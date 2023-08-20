//
// Created by fagod on 22/07/2023.
//

#ifndef DISCORD_BOT_DBADAPTER_H
#define DISCORD_BOT_DBADAPTER_H

#include <tavernbot/tavernbot.h>

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>

using std::map;
using std::pair;
using std::string;
using std::vector;

struct GuildRole {
  uint64_t role_id = 0;
  int64_t points_threshold = 0;
  bool is_best_in_text = false;
  bool is_best_in_voice = false;
};

class DBAdapter {
public:
  explicit DBAdapter(const string &db_path);

  using u_points =
      map<uint64_t, pair<uint32_t, uint32_t>>; /*message_points, text_points*/

  void start_time_count(const uint64_t &user_id, const uint64_t &guild_id);

  void stop_time_count(const uint64_t &user_id, const uint64_t &guild_id);

  void flush_time_count();

  void write_message_info(const uint64_t &user_id, const uint64_t &guild_id,
                          const string &msg, bool has_attachments);

  bool in_connected(const uint64_t &user_id, const uint64_t &guild_id);

  void add_to_white_list(const uint64_t &guild_id, const uint64_t &channel_id);

  void delete_from_white_list(const uint64_t &guild_id,
                              const uint64_t &channel_id);
  void add_role(const uint64_t &guild_id, const uint64_t &role_id,
                const int64_t &points_threshold, const bool is_best_in_text = false,
                const bool is_best_in_voice = false);
  void delete_role(const uint64_t &guild_id, const uint64_t &role_id);

  bool in_whitelist(const uint64_t &guild_id, const uint64_t &channel_id);

  const DBAdapter::u_points *calculate_user_points(const uint64_t &guild_id);

  const map<uint64_t, vector<GuildRole>> &get_roles() const;

private:
  void cash_white_list();
  void cash_roles();

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
  map<uint64_t, vector<GuildRole>> roles_;
  map<uint64_t, u_points> user_points_;

  uint32_t SECOND_COST;
  uint32_t WORD_COST;
  uint32_t ATTACHMENT_COST;
  uint32_t TRACKED_PERIOD;
};

#endif // DISCORD_BOT_DBADAPTER_H
