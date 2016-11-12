#!/bin/bash

BASE=$(pwd)
SUBMISSIONS=$BASE/submissions
BENCHMARK=$BASE/benchmark
RESULTS=$BASE/results
LIBRARY=$BASE/library
MODULE=$BASE/kernel_module

# Parse input
if [ $# -ne 1 ]; then
    echo "Usage: $0 <inputfile>"
    exit
fi

input=$(echo $1 | sed 's:.*/::g')

if [ ! -e $SUBMISSIONS/$input ]; then
    echo "Input file should be file name in $SUBMISSIONS"
    exit
fi

# Create Directories
echo "Creating directories"
if [ ! -d "/usr/local/include/keyvalue" ]; then
    sudo mkdir /usr/local/include/keyvalue
fi
if [ ! -d $RESULTS ]; then
    sudo mkdir $RESULTS
fi

# Setup library path
export LD_LIBRARY_PATH=/usr/local/lib

# Copy input file into kernel_module
echo "Copying $input into kernel_module"
if [ -e kernel_module/keyvalue.c ]; then
    sudo rm $MODULE/keyvalue.c
fi
cp $SUBMISSIONS/$input $MODULE/keyvalue.c

# Create output file
output=$RESULTS/$(echo $input | sed 's/\.c/.txt/g')
echo "Creating $output"
if [ -e $output ]; then
    sudo rm $output
fi
echo "" > $output

# Compile kernel module
echo "Compiling kernel_module: $input"
cd $MODULE
sudo make clean > /dev/null 2>&1
sudo make > /dev/null 2>&1
sudo make install > /dev/null 2>&1
if [ ! -e $MODULE/keyvalue.ko ]; then
    echo -e "0\tCompile failed." >> $output
fi
sudo insmod $MODULE/keyvalue.ko > /dev/null 2>&1
if [ ! -e /dev/keyvalue ]; then
    echo -e "0\tInstall failed." >> $output
fi 
cd $BASE

# Compile library
echo "Compiling library"
cd $LIBRARY
sudo make clean > /dev/null 2>&1
sudo make all install > /dev/null 2>&1
cd $BASE

# Compile testing code
echo "Compiling tests"
cd $BENCHMARK
make all > /dev/null 2>&1
cd $BASE

echo "Testing $input"

echo "Test 1"
sudo insmod $MODULE/keyvalue.ko > /dev/null 2>&1
$BENCHMARK/tests 1 >> $output 2>/dev/null
sudo rmmod keyvalue > /dev/null 2>&1

echo "Test 2"
sudo insmod $MODULE/keyvalue.ko > /dev/null 2>&1
$BENCHMARK/tests 2 >> $output 2>/dev/null
sudo rmmod keyvalue > /dev/null 2>&1

echo "Test 3"
sudo insmod $MODULE/keyvalue.ko > /dev/null 2>&1
$BENCHMARK/tests 3 >> $output 2>/dev/null 
sudo rmmod keyvalue > /dev/null 2>&1

echo "Test 4"
sudo insmod $MODULE/keyvalue.ko > /dev/null 2>&1
$BENCHMARK/tests 4 >> $output 2>/dev/null 
sudo rmmod keyvalue > /dev/null 2>&1

echo "Test 5"
sudo insmod $MODULE/keyvalue.ko > /dev/null 2>&1
$BENCHMARK/tests 5 >> $output 2>/dev/null
sudo rmmod keyvalue > /dev/null 2>&1

echo "Test 6"
sudo insmod $MODULE/keyvalue.ko > /dev/null 2>&1
$BENCHMARK/concurrency 20 1000000
cat output* | sort -k2n > output
$BENCHMARK/validate 20 < output >> $output
rm output1 output2 output3
sudo rmmod keyvalue > /dev/null 2>&1

sum=$(cat $output | awk '{sum+=$1 ;} END{print "TOTAL:",sum}')
echo $sum >> $output

echo -e "\n" >> $output
grep "aawasth2\|kbalakr\|mbhatt\|pbhogat\|jbollab\|svchalla\|achoudh3\|mdai3\|jdeng8\|dndesai\|adeshpa2\|aganji\|jfgiall2\|sgopalk\|agopi\|mgoyal\|agulava\|dguttik\|jhe16\|kjadhav\|ajagana\|jhjain\|sjha5\|akanwar2\|jhkhetwa\|akulkar4\|skunapa\|amajumd\|fsmisarw\|idmunje\|rmyers\|mnagabh\|rnambis\|anayak2\|mohank\|soneil\|kpalani5\|vpalle\|spaluru\|apandey4\|ppatel11\|rpathur\|kspatil2\|rpawar\|sponnam\|rpothir\|kprabhu\|dprasan\|sramach8\|nramesh2\|vravi2\|nsabnis\|sssaha2\|rssakhar\|jsingh8\|asingh27\|rsinha2\|lsole\|rsomsol\|asoni2\|asood3\|ssreeni\|htan5\|kvatwan\|bbangla\|sveluth\|rverma5\|gverma\|pvichar\|avirman2\|jlwheele\|sxia4\|sazawad\|wzhou9" $SUBMISSIONS/$input >> $output

cat $output

