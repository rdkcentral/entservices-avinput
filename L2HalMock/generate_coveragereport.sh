#!/bin/bash

# Define ANSI color codes for green
GREEN='\033[0;32m'     # Green text
NC='\033[0m'           # No color (resets to default)

SCRIPT=$(readlink -f "$0")
SCRIPTS_DIR=`dirname "$SCRIPT"`
RDK_DIR=$SCRIPTS_DIR/../

echo -e "${GREEN}======================== Kill WPEFramework ====================================${NC}"
killall -QUIT WPEFramework
sleep 5
echo -e "${GREEN}======================== Generate coverage Report ====================================${NC}"
cp ${RDK_DIR}/Tests/L2HALMockTests/Test_Framework/.lcovrc_halmock ~/.lcovrc

lcov -c -o coverage.info -d ${RDK_DIR}/build/
lcov -r coverage.info '/usr/include/*' '*/install/usr/include/*' '*/ccec/*' '*/helpers/*' -o filtered_coverage.info

genhtml -o coverage -t "HALMock RDKServices Coverage" filtered_coverage.info