#!/bin/bash

#Fetch the Args
GivenPlugins="$1"

echo "Starting Services for Plugins: $GivenPlugins"

echo "Found: $GivenPlugins"


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

# Function to wait for a port to become available
wait_for_port() {
    local port="$1"
    local timeout="$2"

    for ((i=0; i<timeout; i++)); do
        if nc -z -w 1 localhost "$port"; then
            echo "Port $port is listening. Your process is up and running."
            return 0
        fi

        if [ $i -eq $((timeout - 1)) ]; then
            echo "Timeout: Port $port is not listening after waiting for $timeout seconds."
            return 1
        fi

        sleep 1
    done
}

cp -r /usr/lib/x86_64-linux-gnu/dbus-1.0 /usr/lib/aarch64-linux-gnu/
mv  /usr/share/dbus-1/system.conf /usr/share/dbus-1/system.conf_org_1
mv  /usr/share/dbus-1/session.conf /usr/share/dbus-1/session.conf_org_1
DBUS_CONF_DIR=$SCRIPTS_DIR/dbus
cp $DBUS_CONF_DIR/system.conf /usr/share/dbus-1/system.conf
cp $DBUS_CONF_DIR/session.conf /usr/share/dbus-1/session.conf
service dbus restart

echo -e "${GREEN}========================================Stop all existing services===============================================${NC}"
#Stop flask
fuser -k ${flask_port}/tcp
killall -9 python3
killall -9 IARMDaemonMain
killall -9 CecDaemonMain
killall -9 WPEFramework
killall -9 dsSrvMain
killall -9 pwrSrvMain

pkill python3
pkill IARMDaemonMain
pkill CecDaemonMain
pkill WPEFramework
pkill dsSrvMain
pkill pwrSrvMain

echo -e "${GREEN}========================================flask dependency===============================================${NC}"
if grep -q "HdmiCecSource" <<< "$GivenPlugins"; then
    echo "Found: HdmiCecSource"
    cd $WORKSPACE/deps/rdk/flask/Flask/
    python3 startup.py &
    wait_for_port "$flask_port" "$timeout_duration"
elif grep -q "HdmiCecSink" <<< "$GivenPlugins"; then
    echo "Found: HdmiCecSink"
    cd $WORKSPACE/deps/rdk/flask/Flask/
    python3 startup.py &
    wait_for_port "$flask_port" "$timeout_duration"
fi
echo -e "${GREEN}========================================Continuing with the rest of the script===============================================${NC}"

echo -e "${GREEN}========================================Run iarmbus===============================================${NC}"
cd $WORKSPACE
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib/
deps/rdk/iarmbus/install/bin/IARMDaemonMain &

unset LD_LIBRARY_PATH

echo -e "${GREEN}========================================Run cec deamon===============================================${NC}"
cd $WORKSPACE
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib/:$WORKSPACE/deps/rdk/hdmicec/install/lib:$WORKSPACE/deps/rdk/hdmicec/ccec/drivers/test:$WORKSPACE/deps/rdk/iarmbus/install/
deps/rdk/hdmicec/install/bin/CecDaemonMain &

echo -e "${GREEN}========================================Run dsSrvMain===============================================${NC}"
cd $WORKSPACE
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib/:$WORKSPACE/deps/rdk/iarmbus/install:$WORKSPACE/deps/rdk/devicesettings/install/lib
$WORKSPACE/deps/rdk/iarmmgrs/dsmgr/dsSrvMain &

echo -e "${GREEN}========================================Run rdkservices===============================================${NC}"
cd $WORKSPACE
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH/usr/local/lib/:$WORKSPACE/deps/rdk/hdmicec/install/lib:$WORKSPACE/deps/rdk/hdmicec/ccec/drivers/test:$WORKSPACE/deps/rdk/iarmbus/install/:$WORKSPACE/install/usr/lib:$WORKSPACE/deps/rdk/devicesettings/install/lib
$WORKSPACE/install/usr/bin/WPEFramework -f -c $WORKSPACE/install/etc/WPEFramework/config.json &
