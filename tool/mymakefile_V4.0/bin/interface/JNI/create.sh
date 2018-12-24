#!/bin/sh
if [ "$#" -ne "1" ]; then
    echo "usage: $0 <add java file name as source create.sh test.java>"
	return
fi
if [ ! -f $1 ] ;
then
	echo "$1 file is no exist!"
	return
fi
echo $1
mac2unix $1
dos2unix $1
classpath=`cat $1 | grep package | sed '1,1s/package //g' | sed '1,1s/;*//g' | sed '1,1s/\./\//g'`
if [ $classpath ] ;
then

mkdir $classpath -p
cp $1 $classpath
name=`echo $classpath | sed '1,1s/\//\./g'`.`echo $1 | sed '1,1s/\.java//g'`
javac $classpath/$1
echo "create `echo $1 | sed '1,1s/\.java//g'`.class suc"
javah -d ../include -jni $name
echo "create ../include/$name.h suc"

else

javac $1
echo "create `echo $1 | sed '1,1s/\.java//g'`.class suc"
name=`echo $1 | sed '1,1s/\.java//g'`
javah -d ../include -jni $name
echo "create ../include/$name.h suc"

fi
