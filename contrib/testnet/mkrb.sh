#!/bin/bash

. config

echo "[+] Creating ratbox instance $1:$2"

# create a ratbox instance, $1 - name, $2 - port, $3 - "host:port:mask" list of connects {}
# note that mask is the ratbox form, ie *.cz or simply not present
sid=`echo $2 | cut -b 3-`
fn=$1.ratbox.conf
cat <<_eof > $fn
serverinfo { name = "$1"; sid = "$SIDBASE$sid"; description = "$1 ratbox test server"; network_name = "IRCNet"; hub = yes; };
listen { port = $2; };
class "users" { ping_time = 2 minutes; number_per_ident = 2; number_per_ip = 3; number_per_ip_global = 5; number_per_cidr = 4; max_number = 100; sendq = 100 kbytes; };
class "server" { ping_time = 5 minutes; connectfreq = 5 seconds; max_number = 10; sendq=2 megabytes; };
auth { user = "*@*"; class = "users"; };
operator "$OPERNAME" { user = "*@127.0.0.1"; password = "nasrat"; flags =  global_kill, remote, kline, unkline, gline,die, rehash, admin, xline, operwall, ~encrypted;};
channel { delay = 30; reop = 1; };
general { throttle_count = 9999; collision_fnc = yes; burst_away = yes;
oper_only_umodes = bots, cconn, debug, full, skill, nchange, rej, spy, external, operwall, locops, unauth;
oper_umodes = locops, servnotice, operwall, wallop;
};
connect "service.$1" { host = "127.0.0.1"; accept_password = "secret"; send_password = "secret"; class = "server"; flags = service; };
connect "static.irc.cz" { host = "127.0.0.1"; accept_password = "secret"; send_password = "secret"; class = "server"; hub_mask = "*"; };
.include "`pwd`/../../doc/sch.conf"

_eof
for i in $3; do
	host=`echo $i | cut -f 1 -d ":"`
	port=`echo $i | cut -f 2 -d ":"`
	mask=`echo $i | cut -f 3 -d ":"`
	echo "connect \"$host\" {" >> $fn
	echo "host = \"127.0.0.1\";" >> $fn
	echo "send_password = \"$PASS\";" >> $fn
	echo "accept_password = \"$PASS\";" >> $fn
	if [ ! -z "$port" ]; then
		echo "  >>> Adding connect to $host:$port (mask:$mask)"
		echo "port = $port;" >> $fn
		echo "flags = autoconn;" >> $fn
	fi
	echo "hub_mask = \"*\";" >> $fn
	echo "class = \"server\";" >> $fn
	echo "};" >> $fn
done


