#!/usr/bin/bash


# echo "$0 $1 $2"
FILESDIR=$1
SEARCHSTR=$2
FILES_BELOW="files_before_filter"
FILES_SEARCHED="files_after_filter"

if [ $# -lt 2 ]
then
    echo "Need at least two parameters"
    exit 1
fi

if [ ! -d $FILESDIR ]
then
    echo "$1 is not a directory"
    exit 1
fi
 
 
touch $FILES_SEARCHED
touch $FILES_BELOW

find $FILESDIR -type f > $FILES_BELOW
# ls $FILESDIR -l > $FILES_BELOW
# echo "All files below $1:"
cat $FILES_BELOW
NUMBER_FILES=$( wc -l $FILES_BELOW | awk '{print $1;}' )

grep $SEARCHSTR $FILESDIR -r > $FILES_SEARCHED
# echo "All searched files:"
cat $FILES_SEARCHED
NUMBER_SEARCHED=$( wc -l $FILES_SEARCHED | awk '{print $1;}' )

echo "The number of files are $NUMBER_FILES and the number of matching lines are $NUMBER_SEARCHED"
