#!/bin/bash
INTERNALS="$1"
diff -u -U0 "$INTERNALS" - | grep -v "^+++ -" | grep "^+" | sed 's-^+--g'
