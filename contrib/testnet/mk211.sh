#!/bin/bash

. config

echo "[+] Creating irc2.11.* instance $1:$2"

# create 2.11 instance, $1 - name, $2 - port, $3 - "host:port:mask host2:port2:mask2" list of c/ns
# sid suffix is last 2 chars from port
sid=`echo $2 | cut -b 3-`
fn=$1.211.conf
cat <<_eof > $fn
M:$1::$1 test server:$2:$SIDBASE$sid
A:NFX tech workgroup:tech@nfx.cz:Client Server::IRCnet:
P::::$2::
Y:222:90:5:10:200000000::
Y:12:90::9999:2048000:2.4:0.8:
S:127.0.0.1:secret:alis:0xD000:12
O:*@127.0.0.1:$PASS:$OPERNAME::12:ApP&:
I:*@*::::12::
_eof
for i in $3; do
	host=`echo $i | cut -f 1 -d ":"`
	port=`echo $i | cut -f 2 -d ":"`
	mask=`echo $i | cut -f 3 -d ":"`
	echo "  >>> Adding connect to $host:$port (mask:$mask)"
	echo "C:127.0.0.1:$PASS:$host:$port:222::" >> $fn
	echo "N:127.0.0.1:$PASS:$host:$mask:222::" >> $fn
	echo "H:*::$host::" >> $fn
done

