#!/bin/sh

# here's how it should look like in the end:
#N canvas 195 82 450 300 com 1;
#X text 100 20 one;
#X text 100 40 two;
#X text 100 60 three;
#X restore 176 77 pd com;

ypos=20

echo "#N canvas 195 82 450 300 com 1;"

for i in `cat $1` ; do 
    echo "#X text 100 $ypos $i;" ;
    let ypos+=20
done

echo "#X restore 176 77 pd com;"
