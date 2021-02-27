#!/bin/sh

BLDDIR=`pwd`
CPPC=g++
CFLAGS="-O -I$BLDDIR/hfa"

LINK=g++
XTRALIBS="-lm -lpthread"

for FILE in hfa/*.cpp ; do
  echo cd `dirname $FILE`\; $CPPC -c $CFLAGS  `basename $FILE`
  (cd `dirname $FILE`; $CPPC -c $CFLAGS  `basename $FILE`)
done
