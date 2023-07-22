mkdir "libs"
# shellcheck disable=SC2164
cd ./libs
git clone https://github.com/brainboxdotcc/DPP.git
git clone https://github.com/gabime/spdlog.git
git clone https://github.com/adeharo9/cpp-dotenv.git
# shellcheck disable=SC2103
cd ..