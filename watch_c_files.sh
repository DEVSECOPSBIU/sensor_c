#!/bin/bash

WATCH_DIR="/home/liezrowice/parasoft/workspace_BIU_TEST/sensor_c"
SCRIPT_PATH="$WATCH_DIR/auto_git_push.sh"

echo "🔍 Watching for changes in C files in $WATCH_DIR..."

while true; do
    inotifywait -m -e modify --format '%w%f' "$WATCH_DIR" | while read -r file; do
        if [[ "$file" =~ \.c$ ]]; then
            echo "📝 Change detected in: $file"
            if [[ -x "$SCRIPT_PATH" ]]; then
                bash "$SCRIPT_PATH" "$file"
            else
                echo "❌ Error: $SCRIPT_PATH not found or not executable!"
            fi
        fi
    done
done
