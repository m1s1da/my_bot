sudo yum install opus-devel
sudo yum install wget
wget -O dpp.rpm https://dl.dpp.dev/latest/linux-x64/rpm
sudo yum localinstall dpp.rpm
rm dpp.rpm
sudo dnf install spdlog
./init.sh
./install_SQLiteCpp.sh