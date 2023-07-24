//
// Created by fagod on 22/07/2023.
//

#include "DBAdapter.h"
#include <chrono>
#include <iostream>
#include "dotenv.h"
#include "spdlog/spdlog.h"


DBAdapter::DBAdapter() {
    auto &dotenv = dotenv::env.load_dotenv("../.env", true);
    if (!dotenv["MIN_MSG_SIZE"].empty()) {
        min_msg_size_ = stoi(dotenv["MIN_MSG_SIZE"]);
    }
    storage_.sync_schema();
    spdlog::info("Database ready");
}

uint64_t DBAdapter::get_time_now() {
    const auto ptr = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(
            ptr.time_since_epoch()).count();
}

// TODO: вызывать пару методов start/stop по таймауту
void DBAdapter::start_time_count(const uint64_t &user_id, const uint64_t &guild_id) {
    user_connected_timestamp_map_[{user_id, guild_id}] = get_time_now();
    spdlog::debug("start_time_count {0}, {1}:  time: {2}", user_id, guild_id, get_time_now());
}

void
DBAdapter::write_time_overall(const uint64_t &user_id, const uint64_t &guild_id, const uint64_t &session_time_length) {
    user_time_overall_map_[{user_id, guild_id}] += session_time_length;
    spdlog::debug("write_time_overall {0}, {1}:  user_time_overall: {2}", user_id, guild_id,
                  user_time_overall_map_[{user_id, guild_id}]);
}

void DBAdapter::stop_time_count(const uint64_t &user_id, const uint64_t &guild_id) {
    auto it = user_connected_timestamp_map_.find({user_id, guild_id});
    if (it == user_connected_timestamp_map_.end()) {
        spdlog::error("ERROR in stop_time_count {0}, {1}: not found!", user_id, guild_id);
        return;
    }
    const uint64_t session_time_length = get_time_now() - it->second;
    user_connected_timestamp_map_.erase(it);
    write_time_overall(user_id, guild_id, session_time_length);
    spdlog::debug("stop_time_count {0}, {1}:  session_time_length: {2}", user_id, guild_id, session_time_length);
}

void DBAdapter::write_message_info(const uint64_t &user_id, const uint64_t &guild_id, const string &msg, bool
has_attachments) {
    if (msg.size() > min_msg_size_) {
        user_long_msg_[{user_id, guild_id}]++;
        spdlog::debug("long message received from: {0}, {1}", user_id, guild_id);
    } else {
        user_short_msg_[{user_id, guild_id}]++;
        spdlog::debug("short message received from: {0}, {1}", user_id, guild_id);
    }
}

bool DBAdapter::in_connected(const uint64_t &user_id, const uint64_t &guild_id) {
    return user_connected_timestamp_map_.find({user_id, guild_id}) != user_connected_timestamp_map_.end();
}

DBAdapter::~DBAdapter() = default;
