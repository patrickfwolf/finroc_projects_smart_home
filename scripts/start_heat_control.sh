#!bin/bash

cd ~/finroc
source scripts/setenv -p smart_home
sudo HeatingControl -c sources/cpp/projects/smart_home/etc/config.xml &
cd -
