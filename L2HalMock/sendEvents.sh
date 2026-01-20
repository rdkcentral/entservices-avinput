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
SENDER=$WORKSPACE/deps/rdk/sender/

echo -e "${GREEN}========================================Run IARM_event_sender===============================================${NC}"
cd $SENDER
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib:$WORKSPACE/deps/rdk/iarm_event_sender:$WORKSPACE/deps/rdk/iarmbus/install
./IARM_event_sender cecmgrstatus 
./IARM_event_sender dsmrghotplug 0
./IARM_event_sender dsmrghotplug 1
./IARM_event_sender dsmrgInhotplug 0
./IARM_event_sender dsmrgInhotplug 1
./IARM_event_sender powerModeUpdate 0 1
./IARM_event_sender hdcpstatusEvent 1


