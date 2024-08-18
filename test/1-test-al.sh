./bin/sclc test/1-test-al.in test/1-test-al.out --lexical_only > test/1-test-al.out.txt
if ! cmp -s test/1-test-al.out.txt test/1-test-al-expected-output.txt; then
  echo "Output does not match expected"
  exit 1
fi