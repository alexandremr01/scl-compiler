./bin/sclc test/3-test-semantic.in test/3-test-semantic.out > test/3-test-semantic.out.txt
if ! cmp -s test/3-test-semantic.out.txt test/3-test-semantic-expected-output.txt; then
  echo "Output does not match expected"
  exit 1
fi