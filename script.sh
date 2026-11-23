#!/bin/bash

BIN=./bin/riscv-sim
pass=0
fail=0

for in in $(find tests -name "*.bin" | sort); do
  exp="${in%.bin}.res"
  [ -f "$exp" ] || continue

  out=$(mktemp)
  timeout 5 $BIN "$in" "$out" > /dev/null 2>&1

  if cmp -s "$out" "$exp"; then
    echo "PASS: $in"
    pass=$((pass + 1))
  else
    echo "FAIL: $in ---------------"
    fail=$((fail + 1))
  fi

  rm -f "$out"
done

echo "Passed: $pass, Failed: $fail"