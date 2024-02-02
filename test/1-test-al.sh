./bin/scl test/1-test-al.in --lexical_only > test/1-test-al-output.out
if ! cmp -s test/1-test-al-output.out test/1-test-al-expected-output.txt; then
  echo "Output does not match expected"
  exit 1
fi