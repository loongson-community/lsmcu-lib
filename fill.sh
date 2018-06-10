#!/bin/bash
FILEIN=openloongson1c.bin
FILEOUT=flash.bin
TARGET_SIZE=$((512 * 1024))
FILESIZE=$(stat -c%s "$FILEIN")
val=`expr $TARGET_SIZE - $FILESIZE`
echo $TARGET_SIZE
echo $FILESIZE
cp $FILEIN $FILEOUT
dd if=/dev/zero bs=1 count=$val >> $FILEOUT
