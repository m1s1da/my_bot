//
// Created by fagod on 24/07/2023.
//

#ifndef DISCORD_BOT_DBCREATOR_H
#define DISCORD_BOT_DBCREATOR_H
#include <sqlite_orm/sqlite_orm.h>
#include <string>

using std::string;

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

using namespace sqlite_orm;

auto make_storage_func() {
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

#endif //DISCORD_BOT_DBCREATOR_H
