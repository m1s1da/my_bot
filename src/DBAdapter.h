//
// Created by fagod on 22/07/2023.
//

#ifndef DISCORD_BOT_DBADAPTER_H
#define DISCORD_BOT_DBADAPTER_H

#include <map>
#include <cstdint>
#include <utility>
#include <string>
#include <memory>

#include "sqlite3.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

using std::string;

class DBAdapter {
public:
    DBAdapter(const string &db_path);

    using ug_pair = std::pair<uint64_t, uint64_t>;

    void start_time_count(const uint64_t &user_id, const uint64_t &guild_id);

    void stop_time_count(const uint64_t &user_id, const uint64_t &guild_id);

    void flush_time_count();

    void write_message_info(const uint64_t &user_id, const uint64_t &guild_id, const string &msg, bool has_attachments);

    bool in_connected(const uint64_t &user_id, const uint64_t &guild_id);

    void add_white_list(const uint64_t &guild_id, const uint64_t &channel_id);

    virtual ~DBAdapter();

private:
    void write_time_overall(const uint64_t &user_id, const uint64_t &guild_id, const uint64_t &session_time_length);

    static uint64_t get_time_now();

private:
    std::map<ug_pair, uint64_t> user_connected_timestamp_map_;

    sqlite3 *db_ = nullptr;
    std::shared_ptr<spdlog::logger> err_logger_;
};

#endif //DISCORD_BOT_DBADAPTER_H
