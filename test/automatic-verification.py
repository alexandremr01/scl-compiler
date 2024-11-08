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

def parse_last_line(file_path):
    with open(file_path, 'r') as file:
        lines = file.readlines()[-1]
    return lines.strip()

def compile_and_simulate(input_file, is_float=False):
    debug_cmd = 'debug_float' if is_float else 'debug'
    commands = [
        f'../bin/sclc {input_file} {input_file}.bin --dialect=ABI',
        f'riscv64-unknown-linux-gnu-objcopy -I binary -O elf64-littleriscv -B riscv {input_file}.bin {input_file}.obj',
        f'riscv64-unknown-linux-gnu-ld -T loader.ld -o {input_file}.elf  {input_file}.obj',
        f'spike -d --isa=RV64IMF -m0x10000:0x12000 --pc=0x11000 --debug-cmd={debug_cmd} {input_file}.elf 2> {input_file}.log.txt'
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

def e2e_test(input_file, validation, verbose, is_float):
    compile_and_simulate(input_file, is_float)    
    log_filename = f'{input_file}.log.txt'
    if is_float:
        registers = parse_last_line(log_filename)
    else:
        registers = parse_registers(log_filename)
    if verbose:
        print(input_file)
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
    },
    {
        "file": "matrices.in",
        "assertion": (lambda registers : ('a0' in registers and registers['a0'] == 18))
    },
    {
        "file": "mlp0.in",
        "assertion": (lambda registers : ('a0' in registers and registers['a0'] == 3))
    },
    {
        "file": "relupos.in",
        "assertion": (lambda registers : registers == '2.5'),
        "is_float": True
    },
    {
        "file": "reluneg.in",
        "assertion": (lambda registers : registers == '0'),
        "is_float": True
    }
]

def main():
    verbose = '--verbose' in sys.argv
    for test in TESTS:
        e2e_test(
            test['file'],
            test['assertion'],
            verbose,
            test.get('is_float', False)
        )

if __name__ == "__main__":        
    main()
