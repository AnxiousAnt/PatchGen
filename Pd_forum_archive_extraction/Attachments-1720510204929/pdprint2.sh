#!/bin/sh

echo "pd open $1.pd $2;" | pdsend 3001
wait 1000
echo "pd-$1.pd print $2/$1.ps;" | pdsend 3001
