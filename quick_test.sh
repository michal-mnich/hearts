#!/bin/bash

make &> /dev/null
./kierki-serwer -f rozgrywka.txt -p 12345 &> server.log &
sleep 1
./kierki-klient -h localhost -p 12345 -N -a &> clientN.log &
./kierki-klient -h localhost -p 12345 -W -a &> clientW.log &
./kierki-klient -h localhost -p 12345 -E -a &> clientE.log &
./kierki-klient -h localhost -p 12345 -S -a &> clientS.log &
wait
