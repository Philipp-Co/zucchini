#!/bin/sh
set -e

/usr/local/bin/zucchini_server &
SERVER_PID=$!

trap 'kill "$SERVER_PID" 2>/dev/null' TERM INT

sleep 1

echo "Set up Admin-Stuff..."
/usr/local/bin/zucchini_admin_client whitelist-add 1
/usr/local/bin/zucchini_admin_client whitelist-add 2
/usr/local/bin/zucchini_admin_client whitelist-add 3
/usr/local/bin/zucchini_admin_client whitelist-add 4
/usr/local/bin/zucchini_admin_client status
echo "Admin-Stuff done..."

/usr/local/bin/zucchini_local_client_test
CLIENT_EXIT=$?

kill "$SERVER_PID" 2>/dev/null
wait "$SERVER_PID" 2>/dev/null

exit "$CLIENT_EXIT"
