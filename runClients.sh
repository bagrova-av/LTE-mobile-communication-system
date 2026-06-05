#!/bin/bash

CLIENT_EXEC="./build/TelecomAppClient"
MAX_CLIENTS=10

usage() {
    echo "Usage: $0 [N]"
    echo "  N - number of clients (1-$MAX_CLIENTS, default: 3)"
    echo "Example: $0 5"
}

if [ ! -f "$CLIENT_EXEC" ]; then
    echo "Error: Executable not found at $CLIENT_EXEC. Please build the project first"
    exit 1
fi

NUM_CLIENTS=3

if [ $# -gt 0 ]; then
    if [ "$1" = "-h" ] || [ "$1" = "--help" ]; then
        usage
        exit 0
    fi

    if [[ "$1" =~ ^[0-9]+$ ]]; then
        NUM_CLIENTS=$1
    else
        echo "Error: invalid argument. Please provide a number"
        usage
        exit 1
    fi
fi

if [ "$NUM_CLIENTS" -lt 1 ]; then
    echo "Error: number of clients must be at least 1"
    exit 1
fi

if [ "$NUM_CLIENTS" -gt "$MAX_CLIENTS" ]; then
    echo "Error: cannot start more than $MAX_CLIENTS clients"
    echo "Requested: $NUM_CLIENTS"
    exit 1
fi

launchInTerminal () {
    local cmd="$1"
    local title="$2"

    if command -v gnome-terminal &> /dev/null; then
        gnome-terminal --title="$title" -- bash -c "$cmd; exec bash" > /dev/null 2>&1 &
        disown
    elif command -v konsole &> /dev/null; then
        konsole -e bash -c "$cmd; exec bash" > /dev/null 2>&1 &
        disown
    elif command -v xfce4-terminal &> /dev/null; then
        xfce4-terminal -T "$title" -e "bash -c '$cmd; exec bash'" > /dev/null 2>&1 &
        disown
    elif command -v mate-terminal &> /dev/null; then
        mate-terminal --title="$title" -e "bash -c '$cmd; exec bash'" > /dev/null 2>&1 &
        disown
    elif command -v x-terminal-emulator &> /dev/null; then
        x-terminal-emulator -e "bash -c '$cmd; exec bash'" > /dev/null 2>&1 &
        disown
    else
        echo "Error: no supported terminal emulator found (tried gnome-terminal, konsole, xfce4-terminal, etc.)"
        exit 1
    fi
}

echo "Starting $NUM_CLIENTS client(s)"
for i in $(seq 1 $NUM_CLIENTS); do
    launchInTerminal "$CLIENT_EXEC --config data/config$i.json --nodes node$i" "Client $i"
    sleep 1
done
echo "$NUM_CLIENTS client(s) launched successfully"