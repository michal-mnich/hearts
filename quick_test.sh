#!/bin/bash

make &> /dev/null
./kierki-serwer -f rozgrywka.txt -p 53817 &> server.log &
sleep 1
./kierki-klient -h localhost -p 53817 -N -a &> clientN.log &
./kierki-klient -h localhost -p 53817 -W -a &> clientW.log &
./kierki-klient -h localhost -p 53817 -E -a &> clientE.log &
./kierki-klient -h localhost -p 53817 -S -a &> clientS.log &
wait
