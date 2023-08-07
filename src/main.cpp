#include "dotenv.h"
#include <dpp/dpp.h>

#include "DBAdapter.h"
#include "spdlog/spdlog.h"
#include "ClusterSetter.h"

int main() {
#ifdef NDEBUG
  spdlog::set_level(spdlog::level::info);
#else
  spdlog::set_level(spdlog::level::debug);
#endif

  auto &dotenv = dotenv::env.load_dotenv("../.env", true);
  dpp::cluster bot(dotenv["BOT_TOKEN"],
                   dpp::i_default_intents | dpp::i_message_content);

  DBAdapter db_adapter(dotenv["DB_PATH"]);

  ClusterSetter::setup_cluster(bot, db_adapter);

  bot.start(dpp::st_wait);
}