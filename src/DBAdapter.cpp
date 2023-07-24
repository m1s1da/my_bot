//
// Created by fagod on 22/07/2023.
//

#include "DBAdapter.h"
#include <chrono>
#include <iostream>
#include "dotenv.h"
#include "spdlog/spdlog.h"

using namespace sqlite_orm;
template<>
DBAdapter<>::DBAdapter(sqlite_orm::internal::storage_t<> storage): storage_{storage} {
    auto &dotenv = dotenv::env.load_dotenv("../.env", true);
    if (!dotenv["MIN_MSG_SIZE"].empty()) {
        min_msg_size_ = stoi(dotenv["MIN_MSG_SIZE"]);
    }
    storage_.sync_schema();
}

template<>
uint64_t DBAdapter<>::get_time_now() {
    const auto ptr = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(
            ptr.time_since_epoch()).count();
}

// TODO: вызывать пару методов start/stop по таймауту
template<>
void DBAdapter<>::start_time_count(const uint64_t &user_id) {
    user_connected_timestamp_map_[user_id] = get_time_now();
    std::cout << "start_time_count user_id: " << user_id << " time: " << get_time_now() << std::endl;
}

template<>
void DBAdapter<>::write_time_overall(const uint64_t &user_id, const uint64_t &session_time_length) {
    user_time_overall_map_[user_id] += session_time_length;
    std::cout << "write_time_overall user_id: " << user_id << " user_time_overall: " << user_time_overall_map_[user_id]
              << std::endl;
}

template<>
void DBAdapter<>::stop_time_count(const uint64_t &user_id) {
    auto it = user_connected_timestamp_map_.find(user_id);
    if (it == user_connected_timestamp_map_.end()) {
        std::cout << "ERROR in stop_time_count user_id: " << user_id << " not found!" << std::endl;
        return;
    }
    const uint64_t session_time_length = get_time_now() - it->second;
    user_connected_timestamp_map_.erase(it);
    write_time_overall(user_id, session_time_length);
    std::cout << "stop_time_count user_id: " << user_id << " session_time_length: " << session_time_length << std::endl;
}



template<>
void DBAdapter<>::write_message_info(const uint64_t &user_id, const std::size_t &msg_size) {
    if (msg_size > min_msg_size_) {
        user_long_msg_[user_id]++;
        std::cout << "long message received from: " << user_id <<
                  std::endl;
    } else {
        user_short_msg_[user_id]++;
        std::cout << "short message received from: " << user_id <<
                  std::endl;
    }
}

template<>
bool DBAdapter<>::in_connected(const uint64_t &user_id) {
    return user_connected_timestamp_map_.find(user_id) != user_connected_timestamp_map_.end();
}

template<class T>
DBAdapter<T>::~DBAdapter() = default;
