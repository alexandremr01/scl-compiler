# Simplified C Language (SCL) Compiler

## Setup Docker environment

Scripts `make build-docker` and `make run-docker` are given for easiness of use.

## Build and execute

Use `make all` to build and `./bin/scl filename` to compile. You can run `make test` to run some test scripts.

## Usage

Compile with `./bin/sclc input.csl output`. This will generate the binary `output`. If you also want the assembly code, run `./bin/sclc input.csl output --asm` that will generate both `output` and `output.asm`.

An util to load in the processor memory is provided. Run `./bin/bin2vhdl output output.txt` and it will convert the binary `output` to VHDL code that loads the binary into the memory.

## Standard Library

The Simplified C Standard Library (SCSTL) provides basic commands for the RISC V processor. Pass the flag `-lsstl` to `sclc` and it will automatically include those functions in the generated binary. In order to use them, the CSL file must include the header of those functions - see the corresponding function reference in order to use it.

The library can be extended by adding assembly files to `scstl` and running `make scstl`. This will use the Assembler in `./bin/riscVasm` to build the new object files.