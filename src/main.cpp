#include "ClusterSetter.h"
#include "DBAdapter.h"
#include <tavernbot/tavernbot.h>

int main() {
#ifdef NDEBUG
  spdlog::set_level(spdlog::level::info);
#else
  spdlog::set_level(spdlog::level::debug);
#endif

  json config;
  std::ifstream configfile("config.json");
  configfile >> config;

  dpp::cluster bot(config["BOT_TOKEN"], dpp::i_default_intents |
                                            dpp::i_message_content |
                                            dpp::i_guild_members);

  DBAdapter db_adapter(config["DB_PATH"]);

  ClusterSetter::setup_cluster(bot, db_adapter);

  bot.start(dpp::st_wait);
}