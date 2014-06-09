#!/bin/bash
#
# This script tests the query engine of the tinysearch project.
# Author: Kartik Menon, Zach Kratochvil, Allison Wang
# Date: 5/26/14


make clean >> TestFile.txt 2>&1
server=pierce.cs.dartmouth.edu
# remove prior versions of log
rm -f TestFile.txt

echo "" > TestFile.txt

echo "******Unit testing.*******\n" >> TestFile.txt 2>&1
make clean; make test >> TestFile.txt 2>&1

./test >> TestFile.txt 2>&1

if [ $? -eq 1 ]; then
echo -e "Unit test of maze client failed. Exiting...\n" >> TestFile.txt 2>&1
exit 1

else
return_code=$?
echo -e "All unit tests passed, return code $return_code. Proceeding to test drive.\n\n" >> TestFile.txt 2>&1
fi


echo "****Testing amazing project.****" >> TestFile.txt 2>&1

make clean; make >> TestFile.txt 2>&1

echo -e "Testing improper number of arguments (as specified). Should print error and directions." >> TestFile.txt 2>&1
echo "Four: " >> TestFile.txt 2>&1
AMStartup a b c d >> TestFile.txt 2>&1
echo "Two with -v: " >> TestFile.txt 2>&1
AMStartup a b -v >> TestFile.txt 2>&1
echo -e "----------\n" >> TestFile.txt 2>&1

echo -e "Testing non-numeric first and second arguments. Should print error." >> TestFile.txt 2>&1
echo -e "First: " >> TestFile.txt 2>&1
AMStartup a 2 $server >> TestFile.txt 2>&1
echo -e "----------\n" >> TestFile.txt 2>&1

echo -e "Testing invalid server. Should print error." >> TestFile.txt 2>&1
echo -e "Non pierce-server: cs.dartmouth.edu" >> TestFile.txt 2>&1
AMStartup 5 5 cs.dartmouth.edu >> TestFile.txt 2>&1
echo -e "----------\n" >> TestFile.txt 2>&1

echo -e "Testing out of range first and second arguments. Should print error." >> TestFile.txt 2>&1
echo -e "0s: " >> TestFile.txt 2>&1
AMStartup 0 0 $server >> TestFile.txt 2>&1
echo -e "10s: " >> TestFile.txt 2>&1
AMStartup 10 10 $server >> TestFile.txt 2>&1
echo -e "----------\n" >> TestFile.txt 2>&1



echo "****Testing complete.****" >> TestFile.txt 2>&1
make clean
