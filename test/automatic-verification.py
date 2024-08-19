import subprocess
import sys

def parse_registers(file_path):
    with open(file_path, 'r') as file:
        lines = file.readlines()[-8:]

    registers = {}

    for line in lines:
        parts = line.split()
        for i in range(0, len(parts), 2):
            reg_name = parts[i].strip(':')
            reg_value = parts[i+1]
            registers[reg_name] = int(reg_value, 16)
    return registers

def compile_and_simulate(input_file):
    commands = [
        f'../bin/sclc {input_file} {input_file}.bin',
        f'riscv64-unknown-linux-gnu-objcopy -I binary -O elf64-littleriscv -B riscv {input_file}.bin {input_file}.obj',
        f'riscv64-unknown-linux-gnu-ld -T loader.ld -o {input_file}.elf  {input_file}.obj',
        f'spike -d --isa=RV64I -m0x10000:0x4000 --pc=0x100b0 --debug-cmd=debug {input_file}.elf 2> {input_file}.log.txt'
    ]
    for i, command in enumerate(commands):
        # print(f'Stage {i}')
        # print(command)
        result = subprocess.run(command,
            stdout=subprocess.PIPE, stderr=subprocess.PIPE,
            check=False, shell=True
        )
        if result.returncode != 0:
            print("FATAL: Command finished with error")
            print(command)
            print(result.stdout)
            exit(1)

def e2e_test(input_file, validation, verbose):
    compile_and_simulate(input_file)    
    registers = parse_registers(input_file+'.log.txt')
    if verbose:
        print(registers)
    assert validation(registers)

TESTS = [
    {
        "file": "fibonacci.in",
        "assertion": (lambda registers : ('a0' in registers and registers['a0'] == 21))
    },
    {
        "file": "multiplication.in",
        "assertion": (lambda registers : ('a0' in registers and registers['a0'] == 15))
    }
]

def main():
    verbose = '--verbose' in sys.argv
    for test in TESTS:
        e2e_test(
            test['file'],
            test['assertion'],
            verbose
        )

if __name__ == "__main__":        
    main()
