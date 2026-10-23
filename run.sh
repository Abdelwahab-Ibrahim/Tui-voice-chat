

#!/bin/sh
set -e

cd "$(dirname "$0")"
mkdir -p build DB

g++ -std=c++20 -Iserver -Icommon server/*.cpp common/*.cpp -lsqlite3 -o build/voice_chat_server
exec ./build/voice_chat_server