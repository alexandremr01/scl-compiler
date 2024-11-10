#!/bin/bash
set -o xtrace

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
./bin/sclc "$input_file" "$output_bin" --dialect=ABI \
    --external input input.bin \
    --external weights1 training/weights/weights_0.bin \
    --external bias1 training/weights/bias_0.bin \
    --external weights2 training/weights/weights_1.bin \
    --external bias2 training/weights/bias_1.bin \
    --external weights_output training/weights/weights_2.bin \
    --external bias_output training/weights/bias_2.bin
riscv64-unknown-linux-gnu-objcopy -I binary -O elf64-littleriscv -B riscv "$output_bin" "$output_obj"
riscv64-unknown-linux-gnu-ld -T test/loader.ld -o "$output_elf" "$output_obj"
# spike -d --isa=RV64IMF -m0x10000:0x12000 --pc=0x11000 "$output_elf"
spike -d --isa=RV64IMF -m0x10000:0x12000 --pc=0x11000 "$output_elf" 