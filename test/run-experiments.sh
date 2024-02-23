./bin/sclc test/fibonacci.in test/fibonacci.out
./bin/sclc test/multiplication.in test/multiplication.out

./bin/bin2vhdl test/fibonacci.out test/fibonacci.out.txt
./bin/bin2vhdl test/multiplication.out test/multiplication.out.txt
