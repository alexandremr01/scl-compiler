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
- `--dialect=1` or `--dialect=2` change the RISCV dialect. Dialect 1 (default) uses only x0-x31. Dialect 2 uses pseudo names such as ra, zero, sp, etc.. Binary generation is guaranteed to work with dialect 1.

### Experiments

The experiments from the report are reproducible with `./test/run-experiments.sh`.