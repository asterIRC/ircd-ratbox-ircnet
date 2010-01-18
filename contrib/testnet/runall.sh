#!/bin/bash

. config

rm -f *.pidi

for i in *.ratbox.conf; do $rb -configfile `pwd`/$i -pidfile $i.pidi; done
for i in *.211.conf; do $in -f $i; done
