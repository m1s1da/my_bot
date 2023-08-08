sudo apt install libopus0
sudo apt install wget
wget -O dpp.deb https://dl.dpp.dev/
sudo dpkg -i dpp.deb
rm dpp.deb
sudo apt install libspdlog-dev
./init.sh
./install_SQLiteCpp.sh