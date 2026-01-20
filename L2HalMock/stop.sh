#!/bin/bash


# Define ANSI color codes for green
GREEN='\033[0;32m'     # Green text
NC='\033[0m'           # No color (resets to default)

# Define ANSI color codes for green
GREEN='\033[0;32m'     # Green text
NC='\033[0m'           # No color (resets to default)

SCRIPT=$(readlink -f "$0")
SCRIPTS_DIR=`dirname "$SCRIPT"`
WORKSPACE=$SCRIPTS_DIR/workspace

# Define the port to check and maximum number of seconds to wait
flask_port=8000
timeout_duration=60
echo -e "${GREEN}========================================Stop all existing services===============================================${NC}"
#Stop flask
fuser -k ${flask_port}/tcp
killall -9 python3
killall -9 IARMDaemonMain
killall -9 CecDaemonMain
killall -9 WPEFramework

pkill python3
pkill IARMDaemonMain
pkill CecDaemonMain
pkill WPEFramework
