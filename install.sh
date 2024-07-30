#!/usr/bin/env bash
# Reset
Color_Off='\033[0m'       # Text Reset

# Regular Colors
Black='\033[0;30m'        # Black
Red='\033[0;31m'          # Red
Green='\033[0;32m'        # Green
Yellow='\033[0;33m'       # Yellow
Blue='\033[0;34m'         # Blue
Purple='\033[0;35m'       # Purple
Cyan='\033[0;36m'         # Cyan
White='\033[0;37m'        # White

sudo -v

wget https://github.com/ajTronic/turbofetch/releases/latest/download/turbofetch

sudo mv turbofetch /usr/local/bin/turbofetch

clear
printf "${Purple} ____                              _
/ ___| _   _  ___ ___ ___  ___ ___| |
\___ \| | | |/ __/ __/ _ \/ __/ __| |
 ___) | |_| | (_| (_|  __/\__ \__ \_|
|____/ \__,_|\___\___\___||___/___(_)
\n
${Color_Off}"
printf "Yay! Make sure you have a nerd font installed as your terminal font.\n"
printf "${Yellow}Usage${Color_Off}: ${Cyan}turbofetch${Color_Off}\n\n"