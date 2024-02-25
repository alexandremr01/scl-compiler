./bin/sclc test/fibonacci.in test/fibonacci.out --dialect=ABI
./bin/sclc test/multiplication.in test/multiplication.out --dialect=ABI

./bin/bin2vhdl test/fibonacci.out test/fibonacci.out.txt
./bin/bin2vhdl test/multiplication.out test/multiplication.out.txt
