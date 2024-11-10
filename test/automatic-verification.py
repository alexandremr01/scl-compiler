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

def compile_and_simulate(input_file, runs, is_float=False, externals=None):
    if externals is None:
        externals = {}
    input_file = f'test/{input_file}'
    external_str = ' '.join([f'--external {k} {v}' for k, v in externals.items()])

    float_cmd = "fregs 0 fa0"
    int_cmd = "reg 0"
    debug_cmds =f'''run {runs}
{float_cmd if is_float else int_cmd}
q'''
    with open('test/debug.txt', 'w') as f:
        f.write(debug_cmds)

    commands = [
        f'./bin/sclc {input_file} {input_file}.bin --dialect=ABI {external_str}',
        f'riscv64-unknown-linux-gnu-objcopy -I binary -O elf64-littleriscv -B riscv {input_file}.bin {input_file}.obj',
        f'riscv64-unknown-linux-gnu-ld -T test/loader.ld -o {input_file}.elf  {input_file}.obj',
        f'spike -d --isa=RV64IMF -m0x10000:0x12000 --pc=0x11000 --debug-cmd=test/debug.txt {input_file}.elf 2> {input_file}.log.txt'
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

def e2e_test(input_file, validation, verbose, runs, is_float, externals):
    compile_and_simulate(input_file, runs, is_float, externals)    
    log_filename = f'test/{input_file}.log.txt'
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
    },
    {
        "file": "mlp.in",
        "assertion": (lambda registers : registers == '16.8637'),
        "runs": 500000,
        "is_float": True,
        "externals": {
            'input': 'training/test0.bin',
            'weights1': 'training/weights/weights_0.bin',
            'bias1': 'training/weights/bias_0.bin',
            'weights2': 'training/weights/weights_1.bin',
            'bias2': 'training/weights/bias_1.bin',
            'weights_output': 'training/weights/weights_2.bin',
            'bias_output': 'training/weights/bias_2.bin'
        }
    }
]

def main():
    verbose = '--verbose' in sys.argv
    for test in TESTS:
        e2e_test(
            test['file'],
            test['assertion'],
            verbose,
            test.get('runs', 5000),
            test.get('is_float', False),
            test.get('externals', None),
        )

if __name__ == "__main__":        
    main()
