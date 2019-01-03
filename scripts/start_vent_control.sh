#!bin/bash

cd ~/finroc
source scripts/setenv -p smart_home
VentControl --connect=heatcontrol:4444 &
cd -
