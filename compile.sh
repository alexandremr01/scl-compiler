#!/bin/bash

# Check if an input file is provided
if [ -z "$1" ]; then
  echo "Usage: ./compile.sh <input_file>"
  exit 1
fi

# Extract the base name and directory of the input file
input_file="$1"
base_name="${input_file%.*}"
output_bin="$base_name.out"
output_obj="$base_name.o"
output_elf="$base_name.elf"

# Run each command with the derived file names
./bin/sclc "$input_file" "$output_bin" --dialect=ABI --external var vector.bin
riscv64-unknown-linux-gnu-objcopy -I binary -O elf64-littleriscv -B riscv "$output_bin" "$output_obj"
riscv64-unknown-linux-gnu-ld -T test/loader.ld -o "$output_elf" "$output_obj"
spike -d --isa=RV64IM -m0x10000:0x4000 --pc=0x100b0 "$output_elf"