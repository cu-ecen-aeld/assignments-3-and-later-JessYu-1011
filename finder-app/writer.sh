#!/bin/sh


writefile=$1
writestr=$2
param_num=$#

if [ $param_num != 2 ]; then
	exit 1
fi

mkdir -p "$(dirname "$writefile")" && touch "$writefile"

if [ ! -e $writefile ]; then 
	exit 1
fi

echo "$writestr" > "$writefile"

exit 0
