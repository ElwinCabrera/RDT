#!/bin/bash

#Script created by Elwin Cabrera
#Intended for CSE489 - Modern Networking Concepts, Project 2
#Feel free to modify
#Feel free to share

if [ -e $PWD/run_experiments ] 
  then
   echo ""
  else
    echo "Plese place this script in the same directory as the 'run_experiments' script"
    echo "stopping script so you can go do that"
    exit 1
fi

echo "Enter the path to where the abt, gbn, and sr binaries live"
#echo "Enter an absolute or relative path"
echo "Example- '../<UBIT>' or '~/CSE489/P1/<UBIT>'"
read bin_path

if [ -e $bin_path/abt ] 
  then
    echo -e "\033[0;32mSuccess:\033[0m Found the 'abt' binary in '$bin_path'\033[0m"
  else
    echo -e "\033[1;33mWarning:\033[0m Did not find the 'abt' binary in '$bin_path'\033[0m"
fi

if [ -e $bin_path/gbn ] 
  then
    echo -e "\033[0;32mSuccess:\033[0m Found the 'gbn' binary in '$bin_path'\033[0m"
  else
    echo -e "\033[1;33mWarning:\033[0m Did not find the 'gbn' binary in '$bin_path'\033[0m"
fi    

if [ -e $bin_path/sr ] 
  then
    echo -e "\033[0;32mSuccess:\033[0m Found the 'sr' binary in '$bin_path'\033[0m"
  else
    echo -e "\033[1;33mWarning:\033[0m Did not find the 'sr' binary in '$bin_path'\033[0m"
fi 


if [ -d exp1 ]   
  then 
    # check for all folders recursively - or just delete 
    #rm -r exp1
  else
   mkdir exp1 exp1/abt exp1/gbn exp1/sr
   mkdir exp1/abt/other
   mkdir exp1/gbn/w10 exp1/gbn/w50 exp1/gbn/other
   mkdir exp1/sr/w10 exp1/sr/w50 exp1/sr/other
fi

if [ -d exp2 ]   
  then 
    # check for all folders recursively - or just delete
    #rm -r exp2
  else
   mkdir exp2 exp2/abt exp2/gbn exp2/sr
   mkdir exp2/abt/other
   mkdir exp2/gbn/L0.2 exp2/gbn/L0.5 exp2/gbn/L0.8 exp2/gbn/other
   mkdir exp2/sr/L0.2 exp2/sr/L0.5 exp2/sr/L0.8 exp2/sr/other
fi

num_msg=1000
msg_interval=50
corruption=0.2
echo ""
echo "This script will run two required experiments"
echo ""
echo "All experiments will run with these constants (dependent variables) by default" 
echo "  # messages sent (-m) = $num_msg"
echo "  time between each message (-t) = $msg_interval"
echo "  corruption (-c) = $corruption"
echo ""
echo "All experiments will have two independent variables..."
echo "  Window size (-w)- will vary (10,50,100,200,and 500)"
echo "  Loss (-l) - will vary (0.1, 0.2, 0.4, 0.5 ,0.6, and 0.8)"
echo ""
#read -p "Do you wish to modify these constants? [y/N]" answer
read -p "Press any key to continue" answer

echo "Starting experiments..."
echo ""


echo "Running experiment 1 on 'abt'"
./run_experiments -p $bin_path/abt -m 1000 -t 50 -w 10 -l 0.1 -c 0.2 -o exp1/abt/abt_l0.1c0.2.csv
./run_experiments -p $bin_path/abt -m 1000 -t 50 -w 10 -l 0.2 -c 0.2 -o exp1/abt/abt_l0.2c0.2.csv
./run_experiments -p $bin_path/abt -m 1000 -t 50 -w 10 -l 0.4 -c 0.2 -o exp1/abt/abt_l0.4c0.2.csv
./run_experiments -p $bin_path/abt -m 1000 -t 50 -w 10 -l 0.6 -c 0.2 -o exp1/abt/abt_l0.6c0.2.csv
./run_experiments -p $bin_path/abt -m 1000 -t 50 -w 10 -l 0.8 -c 0.2 -o exp1/abt/abt_l0.8c0.2.csv

echo "Running experiment 1 on 'gbn'  (window = 10 then window = 50) "
./run_experiments -p $bin_path/gbn -m 1000 -t 50 -w 10 -l 0.1 -c 0.2 -o exp1/gbn/w10/gbn_l0.1c0.2w10.csv
./run_experiments -p $bin_path/gbn -m 1000 -t 50 -w 10 -l 0.2 -c 0.2 -o exp1/gbn/w10/gbn_l0.2c0.2w10.csv
./run_experiments -p $bin_path/gbn -m 1000 -t 50 -w 10 -l 0.4 -c 0.2 -o exp1/gbn/w10/gbn_l0.4c0.2w10.csv
./run_experiments -p $bin_path/gbn -m 1000 -t 50 -w 10 -l 0.6 -c 0.2 -o exp1/gbn/w10/gbn_l0.6c0.2w10.csv
./run_experiments -p $bin_path/gbn -m 1000 -t 50 -w 10 -l 0.8 -c 0.2 -o exp1/gbn/w10/gbn_l0.8c0.2w10.csv

./run_experiments -p $bin_path/gbn -m 1000 -t 50 -w 50 -l 0.1 -c 0.2 -o exp1/gbn/w50/gbn_l0.1c0.2w50.csv
./run_experiments -p $bin_path/gbn -m 1000 -t 50 -w 50 -l 0.2 -c 0.2 -o exp1/gbn/w50/gbn_l0.2c0.2w50.csv
./run_experiments -p $bin_path/gbn -m 1000 -t 50 -w 50 -l 0.4 -c 0.2 -o exp1/gbn/w50/gbn_l0.4c0.2w50.csv
./run_experiments -p $bin_path/gbn -m 1000 -t 50 -w 50 -l 0.6 -c 0.2 -o exp1/gbn/w50/gbn_l0.6c0.2w50.csv
./run_experiments -p $bin_path/gbn -m 1000 -t 50 -w 50 -l 0.8 -c 0.2 -o exp1/gbn/w50/gbn_l0.8c0.2w50.csv

echo "Running experiment 1 on 'sr' (window = 10 then window = 50)"
./run_experiments -p $bin_path/sr -m 1000 -t 50 -w 10 -l 0.1 -c 0.2 -o exp1/sr/w10/sr_l0.1c0.2w10.csv
./run_experiments -p $bin_path/sr -m 1000 -t 50 -w 10 -l 0.2 -c 0.2 -o exp1/sr/w10/sr_l0.2c0.2w10.csv
./run_experiments -p $bin_path/sr -m 1000 -t 50 -w 10 -l 0.4 -c 0.2 -o exp1/sr/w10/sr_l0.4c0.2w10.csv
./run_experiments -p $bin_path/sr -m 1000 -t 50 -w 10 -l 0.6 -c 0.2 -o exp1/sr/w10/sr_l0.6c0.2w10.csv
./run_experiments -p $bin_path/sr -m 1000 -t 50 -w 10 -l 0.8 -c 0.2 -o exp1/sr/w10/sr_l0.8c0.2w10.csv

./run_experiments -p $bin_path/sr -m 1000 -t 50 -w 50 -l 0.1 -c 0.2 -o exp1/sr/w50/sr_l0.1c0.2w50.csv
./run_experiments -p $bin_path/sr -m 1000 -t 50 -w 50 -l 0.2 -c 0.2 -o exp1/sr/w50/sr_l0.2c0.2w50.csv
./run_experiments -p $bin_path/sr -m 1000 -t 50 -w 50 -l 0.4 -c 0.2 -o exp1/sr/w50/sr_l0.4c0.2w50.csv
./run_experiments -p $bin_path/sr -m 1000 -t 50 -w 50 -l 0.6 -c 0.2 -o exp1/sr/w50/sr_l0.6c0.2w50.csv
./run_experiments -p $bin_path/sr -m 1000 -t 50 -w 50 -l 0.8 -c 0.2 -o exp1/sr/w50/sr_l0.8c0.2w50.csv




echo "Running experiment 2 on 'abt' (for loss 0.2, 0.5, and 0.8, window does not matter)"
cp exp1/abt/abt_l0.2c0.2.csv exp2/abt/abt_l0.2c0.2 #duplicate exp
./run_experiments -p $bin_path/abt -m 1000 -t 50 -w 10 -l 0.5 -c 0.2 -o exp2/abt/abt_l0.5c0.2.csv
cp exp1/abt/abt_l0.8c0.2.csv exp2/abt/abt_l0.8c0.2.csv # duplicate exp



echo "Running experiment 2 on 'gbn' (window = 10, 50, 100, 200 and 500 for loss 0.2, 0.5 and 0.8)"
cp exp1/gbn/w10/gbn_l0.2c0.2w10.csv exp2/gbn/L0.2/gbn_l0.2c0.2w10.csv
cp exp1/gbn/w50/gbn_l0.2c0.2w50.csv exp2/gbn/L0.2/gbn_l0.2c0.2w50.csv
./run_experiments -p $bin_path/gbn -m 1000 -t 50 -w 100 -l 0.2 -c 0.2 -o exp2/gbn/L0.2gbn_l0.2c0.2w100.csv
./run_experiments -p $bin_path/gbn -m 1000 -t 50 -w 200 -l 0.2 -c 0.2 -o exp2/gbn/L0.2/gbn_l0.2c0.2w200.csv
./run_experiments -p $bin_path/gbn -m 1000 -t 50 -w 500 -l 0.2 -c 0.2 -o exp2/gbn/L0.2/gbn_l0.2c0.2w500.csv


./run_experiments -p $bin_path/gbn -m 1000 -t 50 -w 10 -l 0.5 -c 0.2 -o exp2/gbn/L0.5/gbn_l0.5c0.2w10.csv
./run_experiments -p $bin_path/gbn -m 1000 -t 50 -w 100 -l 0.5 -c 0.2 -o exp2/gbn/L0.5/gbn_l0.5c0.2w100.csv
./run_experiments -p $bin_path/gbn -m 1000 -t 50 -w 200 -l 0.5 -c 0.2 -o exp2/gbn/L0.5/gbn_l0.5c0.2w200.csv
./run_experiments -p $bin_path/gbn -m 1000 -t 50 -w 500 -l 0.5 -c 0.2 -o exp2/gbn/L0.5/gbn_l0.5c0.2w500.csv


cp exp1/gbn/w10/gbn_l0.8c0.2w10.csv exp2/gbn/L0.8/gbn_l0.8c0.2w10.csv
cp exp1/gbn/w50/gbn_l0.8c0.2w50.csv exp2/gbn/L0.8/gbn_l0.8c0.2w50.csv
./run_experiments -p $bin_path/gbn -m 1000 -t 50 -w 100 -l 0.8 -c 0.2 -o exp2/gbn/L0.8/gbn_l0.8c0.2w100.csv
./run_experiments -p $bin_path/gbn -m 1000 -t 50 -w 200 -l 0.8 -c 0.2 -o exp2/gbn/L0.8/gbn_l0.8c0.2w200.csv
./run_experiments -p $bin_path/gbn -m 1000 -t 50 -w 500 -l 0.8 -c 0.2 -o exp2/gbn/L0.8/gbn_l0.8c0.2w500.csv



echo "Running experiment 2 on 'sr' (window = 10, 50, 100, 200, and 500 for loss 0.2, 0.5 and 0.8)"
cp exp1/sr/w10/sr_l0.2c0.2w10.csv exp2/sr/L0.2/sr_l0.2c0.2w10.csv
cp exp1/sr/w50/sr_l0.2c0.2w50.csv exp2/sr/L0.2/sr_l0.2c0.2w50.csv
./run_experiments -p $bin_path/sr -m 1000 -t 50 -w 100 -l 0.2 -c 0.2 -o exp2/sr/L0.2sr_l0.2c0.2w100.csv
./run_experiments -p $bin_path/sr -m 1000 -t 50 -w 200 -l 0.2 -c 0.2 -o exp2/sr/L0.2/sr_l0.2c0.2w200.csv
./run_experiments -p $bin_path/sr -m 1000 -t 50 -w 500 -l 0.2 -c 0.2 -o exp2/sr/L0.2/sr_l0.2c0.2w500.csv


./run_experiments -p $bin_path/sr -m 1000 -t 50 -w 10 -l 0.5 -c 0.2 -o exp2/sr/L0.5/sr_l0.5c0.2w10.csv
./run_experiments -p $bin_path/sr -m 1000 -t 50 -w 100 -l 0.5 -c 0.2 -o exp2/sr/L0.5/sr_l0.5c0.2w100.csv
./run_experiments -p $bin_path/sr -m 1000 -t 50 -w 200 -l 0.5 -c 0.2 -o exp2/sr/L0.5/sr_l0.5c0.2w200.csv
./run_experiments -p $bin_path/sr -m 1000 -t 50 -w 500 -l 0.5 -c 0.2 -o exp2/sr/L0.5/sr_l0.5c0.2w500.csv


cp exp1/sr/w10/sr_l0.8c0.2w10.csv exp2/sr/L0.8/sr_l0.8c0.2w10.csv
cp exp1/sr/w50/sr_l0.8c0.2w50.csv exp2/sr/L0.8/sr_l0.8c0.2w50.csv
./run_experiments -p $bin_path/sr -m 1000 -t 50 -w 100 -l 0.8 -c 0.2 -o exp2/sr/L0.8/sr_l0.8c0.2w100.csv 
./run_experiments -p $bin_path/sr -m 1000 -t 50 -w 200 -l 0.8 -c 0.2 -o exp2/sr/L0.8/sr_l0.8c0.2w200.csv 
./run_experiments -p $bin_path/sr -m 1000 -t 50 -w 500 -l 0.8 -c 0.2 -o exp2/sr/L0.8/sr_l0.8c0.2w500.csv

