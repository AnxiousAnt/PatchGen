#!/bin/sh

name=`basename $1 .pd`

pd -send "pd-$name.pd print $name.ps;" $1
