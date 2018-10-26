#!bin/bash

cd ~/finroc
source scripts/setenv -p smart_home
sudo VentControl -c sources/cpp/projects/smart_home/etc/config.xml --connect=heatcontrol:4444 &
cd -
