#!bin/bash

cd /home/pi/finroc
source scripts/setenv -p smart_home
HeatControl &
cd -
