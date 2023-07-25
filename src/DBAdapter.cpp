//
// Created by fagod on 22/07/2023.
//

#include "DBAdapter.h"
#include <chrono>
#include <iostream>
#include "dotenv.h"
#include "spdlog/spdlog.h"

#include "MessageChecker.h"


DBAdapter::DBAdapter() {
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
    const uint32_t word_counter = MessageChecker::getWordCounter(msg);

//    insert into user_messages
//    values ('test','test', 1, {word_counter}, {has_attachments})
//    on conflict (user_id, guild_id)
//    do update set
//        message_counter = message_counter + 1,
//        word_counter = word_counter + {word_counter},
//        attachment_counter = attachment_counter + {has_attachments};
    storage_.insert(
            into<UserMessages>(),
            columns(&UserMessages::user_id, &UserMessages::guild_id, &UserMessages::message_counter, &UserMessages::word_counter, &UserMessages::attachment_counter),
            values(std::make_tuple(std::to_string(user_id), std::to_string(guild_id), word_counter, 1, has_attachments)),
            on_conflict(&UserMessages::user_id, &UserMessages::guild_id)
                    .do_update(set(assign(&UserMessages::message_counter, add(&UserMessages::message_counter, 1)),
                                   assign(&UserMessages::word_counter, add(&UserMessages::word_counter, word_counter)),
                                   assign(&UserMessages::attachment_counter, add(&UserMessages::attachment_counter, static_cast<uint32_t>(has_attachments))))));
//    auto user_message = storage_.get<UserMessages>(std::to_string(user_id), std::to_string(guild_id));
//    user_message.word_counter += word_counter;
//    user_message.message_counter ++;
//    user_message.attachment_counter += has_attachments;
//    storage_.update(user_message);
//    user_msg_[{user_id, guild_id}] += word_counter;
    spdlog::debug("message received from: {0}, {1} word_counter: {2} msg: {3}", user_id, guild_id, word_counter, msg);

}

bool DBAdapter::in_connected(const uint64_t &user_id, const uint64_t &guild_id) {
    return user_connected_timestamp_map_.find({user_id, guild_id}) != user_connected_timestamp_map_.end();
}

DBAdapter::~DBAdapter() = default;
