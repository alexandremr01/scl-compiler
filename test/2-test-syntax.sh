./bin/sclc test/2-test-syntax.in test/2-test-syntax.out > test/2-test-syntax.out.txt
if ! cmp -s test/2-test-syntax.out.txt test/2-test-syntax-expected-output.txt; then
  echo "Output does not match expected"
  exit 1
fi