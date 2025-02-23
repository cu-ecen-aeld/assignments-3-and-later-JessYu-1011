#!/bin/sh

filesdir=$1
searchstr=$2
param_num=$#

if [ $param_num -lt 2 ]; then
	exit 1
fi

if [ ! -d "$filesdir" ]; then
	exit 1
fi

x=$(ls "$filesdir" |wc -l)
y=$(grep -rnw "$filesdir" -e "$searchstr" | wc -l)

echo "The number of files are $x and the number of matching lines are $y"

exit 0
