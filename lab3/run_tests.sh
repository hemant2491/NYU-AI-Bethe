#!/bin/bash

mkdir -p outs
rm -rf outs/*

# for i in maze publish student student2 ; do ./mdp -tol 0.001 -iter 100 tests/$i.txt > outs/$i.out ; done
./mdp -tol 0.001 -iter 100 tests/maze.txt > outs/maze.out
./mdp -tol 0.001 -iter 100 tests/publish.txt > outs/publish.out
./mdp -tol 0.001 -iter 100 tests/student.txt > outs/student.out
./mdp -tol 0.001 -iter 100 tests/student2.txt > outs/student2.out
./mdp -tol 0.001 -iter 100 -min tests/restaurant.txt > outs/restaurant.out
./mdp -tol 0.001 -iter 100 -df 0.9 tests/teach.txt > outs/teach.out



for i in maze publish student student2 restaurant teach ; do diff tests/$i.out outs/$i.out ; done