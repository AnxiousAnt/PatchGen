#!/bin/bash

# Simple script to list all pd externals under a given path, and optionally print comments from related help file

# Configuration ===============================================

PD_BASE_DIR=/Applications/Pd-extended.app/Contents/Resources
PD_DOC_DIR=$PD_BASE_DIR/doc/5.reference
PD_EXTRA_DIR=$PD_BASE_DIR/extra

GET_COMMENTS=no

# ==============================================================

# Turn on extended globbing
shopt -s extglob
cd $PD_EXTRA_DIR

COUNTER=0

# Iterate over the extras dir and all subdirs
for file in `ls @(|*) | grep pd\_`
do
	# Strip the filename suffix and store the result in NAME
	NAME=`echo $file | cut -d'.' -f1`
	if [ $GET_COMMENTS == "yes" ]
	then
		# Look for corresponding help file
		HELP_PATH=`find $PD_DOC_DIR -name $NAME*`
		if [ -n "$HELP_PATH" ]
			# Get the comments from the pd help patch
			then COMMENTS=`grep text* $HELP_PATH | cut -f5- -d ' ' | sed 's/^/	/g'`
		fi
	fi
		
	# Increment the counter
	let COUNTER=$COUNTER+1	

	# Display the results
	echo "Name: $NAME"
	if [ $GET_COMMENTS == "yes" ]
		then
			echo "Comments:" 
			echo "$COMMENTS";
	fi
done

echo "$COUNTER externals found!"
