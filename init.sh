# shellcheck disable=SC2164
cd ./libs
git submodule init
git submodule update
# shellcheck disable=SC2103
cd ..
touch ".env"
