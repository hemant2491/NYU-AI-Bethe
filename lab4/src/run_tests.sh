#!/bin/bash

mkdir -p tmp/outs
rm -rf tmp/outs/*

# ./learn -train tests/<training_file> [-test <test_file>] -K <nof_nearest_neighbors> -C <Laplacian_correction> [-v] [-d e2|manh] [[x0,y0], [x1,y2], ...]
./learn -train tests/1_knn1.train.txt -test tests/1_knn1.test.txt -K 3 -v > tmp/outs/1_knn1.e2.3.out.txt
./learn -train tests/2_knn2.train.txt -test tests/2_knn2.test.txt -K 3 -v > tmp/outs/2_knn2.e2.3.out.txt
./learn -train tests/3_knn3.train.txt -test tests/3_knn3.test.txt -K 3 -v > tmp/outs/3_knn3.e2.3.out.txt
./learn -train tests/3_knn3.train.txt -test tests/3_knn3.test.txt -K 5 -v > tmp/outs/3_knn3.e2.5.out.txt
./learn -train tests/3_knn3.train.txt -test tests/3_knn3.test.txt -K 7 -v > tmp/outs/3_knn3.e2.7.out.txt
./learn -train tests/4_ex1_train.csv -test tests/4_ex1_test.csv -C 0 > tmp/outs/4_nb1.0.out
./learn -train tests/4_ex1_train.csv -test tests/4_ex1_test.csv -C 0 -v > tmp/outs/4_nb1.0.v.out
./learn -train tests/4_ex1_train.csv -test tests/4_ex1_test.csv -C 1 > tmp/outs/4_nb1.1.out
./learn -train tests/4_ex1_train.csv -test tests/4_ex1_test.csv -C 1 -v > tmp/outs/4_nb1.1.v.out
./learn -train tests/5_ex2_train.csv -test tests/5_ex2_test.csv -C 0 > tmp/outs/5_nb2.0.out
./learn -train tests/5_ex2_train.csv -test tests/5_ex2_test.csv -C 0 -v > tmp/outs/5_nb2.0.v.out
./learn -train tests/5_ex2_train.csv -test tests/5_ex2_test.csv -C 1 > tmp/outs/5_nb2.1.out
./learn -train tests/5_ex2_train.csv -test tests/5_ex2_test.csv -C 1 -v > tmp/outs/5_nb2.1.v.out
./learn -train tests/5_ex2_train.csv -test tests/5_ex2_test.csv -C 2 > tmp/outs/5_nb2.2.out
./learn -train tests/5_ex2_train.csv -test tests/5_ex2_test.csv -C 2 -v > tmp/outs/5_nb2.2.v.out
./learn -train tests/6_km1.txt -d e2 0,0 200,200 500,500 > tmp/outs/6_km1.e2.out.txt
./learn -train tests/6_km1.txt -d manh  0,0 200,200 500,500 > tmp/outs/6_km1.manh.out.txt
./learn -train tests/7_km2.txt -d e2 0,0,0 200,200,200 500,500,500 > tmp/outs/7_km2.e2.out.txt
./learn -train tests/7_km2.txt -d manh 0,0,0 200,200,200 500,500,500 > tmp/outs/7_km2.manh.out.txt


cd tests

echo "Entered $(pwd)"

for i in `ls *.out.txt` ; do echo $i ; diff ./$i ../tmp/outs/$i ; echo ; done
for i in `ls *.out` ; do echo $i ; diff ./$i ../tmp/outs/$i ; echo ; done

cd ../

echo "Back in $(pwd)"
 
