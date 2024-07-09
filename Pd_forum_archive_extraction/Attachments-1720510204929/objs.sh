#!/bin/bash
grep "^#X obj" | cut -d ' ' -f 5 | sed 's-;--g' | sort | uniq
