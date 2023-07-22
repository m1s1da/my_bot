//
// Created by fagod on 22/07/2023.
//

#ifndef DISCORD_BOT_DBADAPTER_H
#define DISCORD_BOT_DBADAPTER_H

#include <map>
#include <cstdint>

class DBAdapter {
public:
    DBAdapter();

    void start_time_count(const uint64_t &user_id);

    void stop_time_count(const uint64_t &user_id);

    void write_message_info(uint64_t &user_id, std::size_t msg_size);

private:
    void write_time_overall(const uint64_t &user_id, const uint64_t &session_time_length);

    static uint64_t get_time_now();

    std::map<uint64_t, uint64_t> user_connected_timestamp_map_;
    std::map<uint64_t, uint64_t> user_time_overall_map_;
    std::map<uint64_t, std::size_t> user_short_msg_;
    std::map<uint64_t, std::size_t> user_long_msg_;
    std::size_t min_msg_size_ = {10};
};


#endif //DISCORD_BOT_DBADAPTER_H
