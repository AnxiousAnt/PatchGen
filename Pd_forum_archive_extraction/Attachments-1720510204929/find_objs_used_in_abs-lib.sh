#!/bin/bash
# By Steffen <stffn@dibidut.dk>

if [$1 = ""]; then
	echo "Usage: $0 path/to/lib/"
else
	grep -G '^#' ${1}/* |
	grep obj |
	cut -d' ' -f5 | 
	cut -d';' -f1 |
	sort |
	uniq 
fi;
