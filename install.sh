#!/bin/bash

git clone https://github.com/ajTronic/turbofetch.git

cd turbofetch

sudo cp turbofetch /usr/local/bin/turbofetch

cd ..
sudo rm -rf turbofetch

clear
echo "Finished!"
echo "Usage: turbofetch"