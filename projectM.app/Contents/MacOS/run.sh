#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
RESOURCES_DIR="${DIR}/../Resources"
export DATADIR_PATH="${RESOURCES_DIR}"

echo "========================================"
echo "projectM Music Visualizer v4.1.4"
echo "========================================"
echo ""
echo "Press H for help menu"
echo "Press CMD+Q to quit"
echo ""

"${DIR}/projectM-bin"

echo ""
echo "projectM exited. Press any key to close this window..."
read -n 1
