//
// Created by fagod on 24/07/2023.
//

#ifndef DISCORD_BOT_MESSAGECHECKER_H
#define DISCORD_BOT_MESSAGECHECKER_H

#include <string>

using std::string;

class MessageChecker {
public:
  static uint32_t getWordCounter(const string &msg);
  static bool isEmoji(const string &word);
};

#endif // DISCORD_BOT_MESSAGECHECKER_H
