#!/bin/bash
dir=$PWD/"src/*"
echo $dir
for f in $dir
do
   if [ -f $f ]
   then
	   echo "$f" "operation" "..."
#       tftp -p -l $f $2
#	   cp $f /mnt/hgfs/goeful/tftp

   fi
done
