#!/bin/bash

cd /home/pi/finroc
source scripts/setenv -p smart_home
fingui sources/cpp/projects/smart_home/etc/control.gui --connect=heatcontrol:4444 --fullscreen &
cd -
