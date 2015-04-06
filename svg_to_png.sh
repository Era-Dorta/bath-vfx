#!/bin/sh
file=${1:?Need filename argument to convert}
sed '/style=/ishape-rendering="crispEdges"' $file > $file~
convert $file~ $(echo $file | sed -e 's/svg$/png/')
rm -f $file~
