#!/bin/bash
# Try to create JIS-encoded channel containing ',' to see if JIS support works as expected
# $Id: jistest.sh 99 2010-01-19 22:14:36Z karel.tuma $

if (echo "USER sd 0 sd :sd"; echo "NICK sd--"; /bin/echo -e "JOIN #\033\$Btest,test2\033(B"; echo QUIT) | nc localhost 6607 | grep "test,test"; then
	echo "JP test succeeded"
else
	echo "JP test failed"
fi

