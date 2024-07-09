#/bin/csh

foreach f (*.pd)
    cp $f $f~
# You can change the '928' value to your (y.resolution-96) here ___ ___
    awk '$2 ~ /canvas/ {print $1, $2, $3,($4>22?$4:22), $5, ($6>928?928:$6),($7=="12;"?"10;":$7), $8, $9} \
	 $2 !~ /canvas/ {print $0}' $f~ > $f    
end
