//
// Created by fagod on 22/07/2023.
//

#include "DBAdapter.h"
#include <chrono>
#include <iostream>
#include "dotenv.h"


#include "MessageChecker.h"

using std::to_string;
DBAdapter::DBAdapter(const string &db_path) {
    err_logger_ = spdlog::stderr_color_mt("stderr");
    if (sqlite3_open(db_path.c_str(), &db_)) {
        spdlog::get("err_logger_")->critical("DB opening error {}", sqlite3_errmsg(db_));
        exit(1);
    } else {
        spdlog::info("DB is ready");
    }

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
    const string query =
            "INSERT INTO user_voice VALUES ("
            "'" + to_string(user_id) + "',"
            "'" + to_string(guild_id) +"', " +
            to_string(session_time_length) + ")"
           "ON CONFLICT (user_id, guild_id) DO UPDATE SET "
           "time_counter = time_counter + " + to_string(session_time_length) + ";";
    char *err = nullptr;
    if (sqlite3_exec(db_, query.c_str(), 0, 0, &err))
    {
        spdlog::get("err_logger_")->error("write_time_overall query error {}", err);
        sqlite3_free(err);
    }
    spdlog::debug("write_time_overall {0}, {1}", user_id, guild_id);
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
    const uint32_t word_counter = MessageChecker::getWordCounter(msg);
    const string query =
            "INSERT INTO user_messages VALUES ("
            "'" + to_string(user_id) + "',"
            "'" + to_string(guild_id) +"', "
            "1, " +
            to_string(word_counter) + ", " +
            to_string(static_cast<int>(has_attachments)) + ")"
            "ON CONFLICT (user_id, guild_id) DO UPDATE SET "
            "message_counter = message_counter + 1,"
            "word_counter = word_counter + " + to_string(word_counter) + ","
            "attachment_counter = attachment_counter + " + to_string(static_cast<int>(has_attachments)) + ";";
    char *err = 0;
    if (sqlite3_exec(db_, query.c_str(), 0, 0, &err))
    {
        spdlog::get("err_logger_")->error("write_message_info query error {}", err);
        sqlite3_free(err);
    }
    spdlog::debug("message received from: {0}, {1} word_counter: {2} msg: {3}", user_id, guild_id, word_counter, msg);
}

bool DBAdapter::in_connected(const uint64_t &user_id, const uint64_t &guild_id) {
    return user_connected_timestamp_map_.find({user_id, guild_id}) != user_connected_timestamp_map_.end();
}

DBAdapter::~DBAdapter() {
    sqlite3_close(db_);
}
