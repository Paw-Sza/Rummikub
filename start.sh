#!/bin/bash
unset SESSION_MANAGER
make &
sleep 2
xterm -e ./server 127.0.0.1 $1  &
sleep 0.1 
xterm -T "Client 0" -e ./client 127.0.0.1 $1  &
sleep 0.1
#xterm -T "Client 1" -e ./client 127.0.0.1 $1  &
sleep 0.1
#xterm -T "Client 2" -e ./client 127.0.0.1 $1  &
sleep 0.1
#xterm -T "Client 3" -e ./client 127.0.0.1 $1  &
sleep 0.1

