#!/bin/bash
DIR="$1"
while read abs ; do
  [[ -f "$DIR/$abs.pd" ]] || echo "$abs"
done
