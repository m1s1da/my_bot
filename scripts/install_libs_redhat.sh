yum install opus #??????
yum install wget
wget -O dpp.rpm https://dl.dpp.dev/latest/linux-x64/rpm
yum localinstall dpp.rpm
rm dpp.rpm
dnf install spdlog
./init.sh
./install_SQLiteCpp.sh