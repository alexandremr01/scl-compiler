./bin/sclc test/2-test-syntax.in output > test/2-test-syntax-output.out
if ! cmp -s test/2-test-syntax-output.out test/2-test-syntax-expected-output.txt; then
  echo "Output does not match expected"
  exit 1
fi