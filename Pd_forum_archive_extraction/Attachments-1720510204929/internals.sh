#!/bin/bash
PDBASEDIR="$1"
(grep -R -E "(class_new|class_addcreator)" "$PDBASEDIR/src/" | cut -d '"' -f 2 && echo list) | sort | uniq | grep -v "^$PDBASEDIR/src/"
