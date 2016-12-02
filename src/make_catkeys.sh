#!/bin/sh
# This script makes the catkeys needed before compiling the binaries

make catkeys
cd locales
catfile=en.catkeys
tempfile=en.catkeys_orig
mv $catfile $tempfile
head -1 $tempfile > $catfile
tail -n +2 $tempfile | sort -k 2 -t$'\t' >> $catfile
settype -t locale/x-vnd.Be.locale-catalog.plaintext $catfile
cd ..
