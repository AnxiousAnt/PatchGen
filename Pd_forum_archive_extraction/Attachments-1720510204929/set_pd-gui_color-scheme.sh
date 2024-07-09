#!/bin/sh
# replaces the default color scheme in pd-gui.tcl with the one in mp_pd-gui_color_scheme.tcl
TMP_FILE='pd-gui_new.tcl'
SRC_FILE='pd-gui.tcl'
COLOR_SCHEME_FILE='mp_pd-gui_color_scheme.tcl'
cp /usr/lib/pd-extended/tcl/$SRC_FILE .

# wc -l=count number of lines in input
TOTAL_LINES=`cat $SRC_FILE | wc -l`
echo $TOTAL_LINES
# grep -e=use pattern -n=prefix line number m1=max 1 match
# cut -d=use ':' instead of TAB -f=select only field 1
# set BEGIN_LINE to the line number of the first occurrence of "# color scheme" in $SRC_FILE
BEGIN_LINE=`grep -n -m1 -e '# color scheme' $SRC_FILE | cut -d : -f 1`
echo $BEGIN_LINE
END_LINE=`grep -n -m1 -e 'mixed_nlet' $SRC_FILE | cut -d : -f 1`
echo $END_LINE
TAIL_LINES=$(($TOTAL_LINES-$END_LINE))

echo 'lines in '$SRC_FILE' : '$TOTAL_LINES
echo 'color scheme runs from line: ' $BEGIN_LINE
echo 'to line: ' $END_LINE
echo 'with ' $TAIL_LINES ' lines remaining.'

head -n $BEGIN_LINE $SRC_FILE > $TMP_FILE
cat $COLOR_SCHEME_FILE >> $TMP_FILE
tail -n $TAIL_LINES $SRC_FILE >> $TMP_FILE
TOTAL_NEW_LINES=`cat $TMP_FILE | wc -l`
if [ $TOTAL_NEW_LINES>=$TOTAL_LINES ];
then
    echo "doing: sudo cp $TMP_FILE /usr/lib/pd-extended/tcl/$SRC_FILE"
    sudo cp $TMP_FILE /usr/lib/pd-extended/tcl/$SRC_FILE
else
    echo "$TOTAL_LINES not equal to $TOTAL_NEW_LINES"
fi
echo "done."

