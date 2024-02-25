./bin/sclc test/3-test-semantic.in output > test/3-test-semantic-output.out
if ! cmp -s test/3-test-semantic-output.out test/3-test-semantic-expected-output.txt; then
  echo "Output does not match expected"
  exit 1
fi