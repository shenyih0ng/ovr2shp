#!/bin/sh

BLDDIR=`pwd`
CC=gcc
CPPC=g++
CFLAGS="-O -I$BLDDIR/hfa -I$BLDDIR/port"

LINK=g++
XTRALIBS="-lm -lpthread"


for FILE in *.c */*.c ; do
  echo cd `dirname $FILE`\; $CC -c $CFLAGS  `basename $FILE`
  (cd `dirname $FILE`; $CC -c $CFLAGS  `basename $FILE`)
done
for FILE in *.cpp */*.cpp ; do
  echo cd `dirname $FILE`\; $CPPC -c $CFLAGS  `basename $FILE`
  (cd `dirname $FILE`; $CPPC -c $CFLAGS  `basename $FILE`)
done
