#!/bin/bash
h=20;
n=1;
echo "#N canvas 408 22 450 628 10;"
echo "#X text 10 0 List of Gem objectglasses in Gem-0.91.0 \"tigital\";"
for i in pix_*-help.pd
do
    echo "#X obj 140 $(($h * $n)) `echo $i | cut -d'-' -f1`;"
    n=$(($n+1))
done
n=1;
for i in part_*-help.pd
do
    echo "#X obj 300 $(($h * $n)) `echo $i | cut -d'-' -f1`;"
    n=$(($n+1))
done
n=1;

for i in `ls *-help.pd | grep -v '^part_' | grep -v '^pix_'`
do
    echo "#X obj 10 $(($h * $n)) `echo $i | cut -d'-' -f1`;"
    n=$(($n+1))
done