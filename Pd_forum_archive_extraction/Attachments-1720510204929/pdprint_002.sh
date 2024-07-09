#!/bin/sh
# print a directory of pd files as postscript
# args: 1: directory of pd patches to print 
# try altering /findfont 9/ to another font size if problems

pd -nomidi -noaudio ./printer.pd &
sleep 2
for i in $1*.pd
do
echo printing $i
sleep 1
echo "pd open $i ./;" | pdsend 3001
sleep 1
echo "pd-$i print $i.ps, menuclose;" | pdsend 3001
sleep 1
awk 'BEGIN  {last = -10}
/findfont 9/  {
        last=NR
        $3 = 11.3
    }
    {
        if (NR == last+2) {
            $1 = $1+1
            $2 = $2-2
        }
        print
    }' $i.ps >  $i-cp.ps
rm $i.ps
mv $i-cp.ps $i.ps
done