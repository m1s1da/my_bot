//
// Created by fagod on 22/07/2023.
//

#ifndef DISCORD_BOT_DBADAPTER_H
#define DISCORD_BOT_DBADAPTER_H

#include <map>
#include <cstdint>
#include <sqlite_orm/sqlite_orm.h>


using std::string;
using namespace sqlite_orm;

struct UserMessages {
    string user_id;
    string guild_id;
    uint32_t message_counter;
    uint32_t word_counter;
    uint32_t embed_counter;
};

struct UserVoice {
    string user_id;
    string guild_id;
    uint32_t voice_timer;
};

auto make_storage_func(){
    return make_storage("../my_bot_db",
                        make_table("user_messages",
                                   make_column("user_id", &UserMessages::user_id),
                                   make_column("guild_id", &UserMessages::guild_id),
                                   make_column("message_counter",
                                               &UserMessages::message_counter),
                                   make_column("word_counter", &UserMessages::word_counter),
                                   make_column("embed_counter", &UserMessages::embed_counter),
                                   primary_key(&UserMessages::user_id,
                                               &UserMessages::guild_id)),
                        make_table("user_voice",
                                   make_column("user_id", &UserVoice::user_id),
                                   make_column("guild_id", &UserVoice::guild_id),
                                   make_column("message_counter", &UserVoice::voice_timer),
                                   primary_key(&UserVoice::user_id, &UserVoice::guild_id))
    );}

class DBAdapter {
public:
    DBAdapter();

    void start_time_count(const uint64_t &user_id);

    void stop_time_count(const uint64_t &user_id);

    void write_message_info(const uint64_t &user_id, const std::size_t &msg_size);

    bool in_connected(const uint64_t &user_id);

    virtual ~DBAdapter();

private:
    void write_time_overall(const uint64_t &user_id, const uint64_t &session_time_length);

    static uint64_t get_time_now();

    std::map<uint64_t, uint64_t> user_connected_timestamp_map_;
    std::map<uint64_t, uint64_t> user_time_overall_map_;
    std::map<uint64_t, std::size_t> user_short_msg_;
    std::map<uint64_t, std::size_t> user_long_msg_;
    std::size_t min_msg_size_ = {10};

    inline static auto storage_{make_storage_func()};
};

#endif //DISCORD_BOT_DBADAPTER_H
