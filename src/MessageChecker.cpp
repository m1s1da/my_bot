//
// Created by fagod on 24/07/2023.
//

#include "MessageChecker.h"

#include <regex>

using std::regex;

uint32_t MessageChecker::getWordCounter(const string &msg) {
  if (msg.empty()) {
    return 0;
  }

  std::regex word_regex("\\S{3,}", std::regex_constants::ECMAScript |
                                       std::regex_constants::icase);

  auto words_begin = std::sregex_iterator(msg.begin(), msg.end(), word_regex);
  auto words_end = std::sregex_iterator();

  uint32_t counter = 0;
  bool has_emoji = false;
  for (std::sregex_iterator it = words_begin; it != words_end; ++it) {
    const std::smatch &match = *it;
    std::string match_str = match.str();
    if (isEmoji(match_str)) {
      has_emoji = true;
    } else {
      counter++;
    }
  }
  counter += has_emoji;

  if (!counter) {
    return 1;
  }

  return counter;
}

bool MessageChecker::isEmoji(const string &word) {
  std::regex emoji_regex("<.{0,10}:\\w+:\\d+>",
                         std::regex_constants::ECMAScript |
                             std::regex_constants::icase);

  auto emojies_begin =
      std::sregex_iterator(word.begin(), word.end(), emoji_regex);
  auto emojies_end = std::sregex_iterator();

  if (emojies_begin != emojies_end) {
    return true;
  }

  return false;
}
