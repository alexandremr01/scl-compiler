# Simplified C Language (SCL) Compiler

## Setup Docker environment

Scripts `make build-docker` and `make run-docker` are given for easiness of use.

## Build and execute

Use `make all` to build and `./bin/scl filename` to compile. You can run `make test` to run some test scripts.

## Usage

Compile with `./bin/sclc input.csl output`. This will generate the binary `output`. If you also want the assembly code, run `./bin/sclc input.csl output --asm` that will generate both `output` and `output.asm`.

An util to load in the processor memory is provided. Run `./bin/bin2vhdl output output.txt` and it will convert the binary `output` to VHDL code that loads the binary into the memory.

### Compiler Flags

- `--asm` outputs the assembly file
- `--asm_comments` adds comments to the assebly file
- `--print_ast` outputs the symbolic tree 
- `--dialect=raw` or `--dialect=ABI` change the register naming. Raw (default) uses only x0-x31. ABI uses names such as ra, zero, sp, etc.. The RISC-V Simulator used only accepts Assembly in ABI naming.

### Experiments

The experiments from the report are reproducible with `./test/run-experiments.sh`.

### Simulating with Spike

```
../bin/sclc test/multiplication.in test/multiplication.out --dialect=ABI
riscv64-unknown-linux-gnu-objcopy -I binary -O elf64-littleriscv -B riscv multiplication.out multiplication.o
riscv64-unknown-linux-gnu-ld -T loader.ld -o multiplication.elf  multiplication.o
spike -d --isa=RV64I -m0x10000:0x4000 --pc=0x100b0 multiplication.elf
run 1000
reg 0
```