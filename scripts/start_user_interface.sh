#!bin/bash

cd /home/pi/finroc
source scripts/setenv -p smart_home
UserInterface &
fingui sources/cpp/projects/smart_home/etc/heatcontrol.gui --fullscreen --connect=heatcontrol:4444 &
cd -
