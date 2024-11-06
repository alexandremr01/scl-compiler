from sklearn.compose import ColumnTransformer
from sklearn.datasets import fetch_openml
from sklearn.pipeline import Pipeline
from sklearn.impute import SimpleImputer
from sklearn.preprocessing import StandardScaler, OneHotEncoder
from sklearn.linear_model import LogisticRegression
from sklearn.model_selection import train_test_split, GridSearchCV

from tqdm import tqdm

import pandas as pd
import numpy as np

import struct
import subprocess

feature=["duration","protocol_type","service","flag","src_bytes","dst_bytes","land","wrong_fragment","urgent","hot",
          "num_failed_logins","logged_in","num_compromised","root_shell","su_attempted","num_root","num_file_creations","num_shells",
          "num_access_files","num_outbound_cmds","is_host_login","is_guest_login","count","srv_count","serror_rate","srv_serror_rate",
          "rerror_rate","srv_rerror_rate","same_srv_rate","diff_srv_rate","srv_diff_host_rate","dst_host_count","dst_host_srv_count",
          "dst_host_same_srv_rate","dst_host_diff_srv_rate","dst_host_same_src_port_rate","dst_host_srv_diff_host_rate","dst_host_serror_rate",
          "dst_host_srv_serror_rate","dst_host_rerror_rate","dst_host_srv_rerror_rate","label","difficulty"]

def prepare(filename, preprocessor=None):
    df = pd.read_csv(filename, names=feature)
    df['is_under_attack'] = np.where(df.label == 'normal', 0, 1)
    df_nolabels = df.drop(labels=['is_under_attack', 'label'], axis=1)
    if preprocessor is None:
        preprocessor = get_processor(df_nolabels)
        df_processed = preprocessor.fit_transform(df_nolabels)
        return df_processed, df['is_under_attack'], preprocessor
    
    return preprocessor.transform(df_nolabels), df['is_under_attack']

def get_processor(df_nolabels):
    numeric_features = df_nolabels.select_dtypes(include='number').columns

    numeric_transformer = Pipeline(steps=[('scaler', StandardScaler())])

    categorical_features = ['protocol_type','service','flag']
    categorical_transformer = OneHotEncoder(handle_unknown='ignore')

    preprocessor = ColumnTransformer(
        transformers=[
            ('num', numeric_transformer, numeric_features),
            ('cat', categorical_transformer, categorical_features)]
    )
    return preprocessor

def save_vector(vector, filename):
    with open(filename, 'wb') as f:
        for number in vector:
             f.write(struct.pack('f', number))


def parse_last_line(file_path):
    with open(file_path, 'r') as file:
        lines = file.readlines()[-1]
    return lines.strip()

if __name__ == '__main__':
    X_train, y_train, preprocessor = prepare('./training/data/KDDTrain+.txt')
    X_test, y_test = prepare('./training/data/KDDTest+.txt', preprocessor)

    num_total = 0
    num_matches = 0
    for ix, row in tqdm(enumerate(X_test[:10]), total=10):
        save_vector(row, 'input.bin')
        commands = [
            f'''./bin/sclc test/mlp.in test/mlp.out --dialect=ABI \
                --external input input.bin \
                --external weights1 training/weights/weights_0.bin \
                --external bias1 training/weights/bias_0.bin \
                --external weights2 training/weights/weights_1.bin \
                --external bias2 training/weights/bias_1.bin \
                --external weights_output training/weights/weights_2.bin \
                --external bias_output training/weights/bias_2.bin''',
            f'riscv64-unknown-linux-gnu-objcopy -I binary -O elf64-littleriscv -B riscv test/mlp.out test/mlp.o',
            f'riscv64-unknown-linux-gnu-ld -T test/loader.ld -o test/mlp.elf test/mlp.o',
            f'spike -d --isa=RV64IMF -m0x10000:0x12000 --pc=0x11000 --debug-cmd=test/debug_mlp test/mlp.elf 2> test/mlp.log.txt'
        ]
        for i, command in enumerate(commands):
            result = subprocess.run(command,
                stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                check=False, shell=True
            )
            if result.returncode != 0:
                print("FATAL: Command finished with error")
                print(command)
                print(result.stdout)
                exit(1)

        value = parse_last_line('test/mlp.log.txt')
        print('Logit ', value, 'actual', y_test[ix])
        prediction = int(float(value) > 0)
        num_total += 1
        num_matches += 1 if prediction == y_test[ix] else 0
    
    print(f'Got {num_matches}/{num_total} ({num_matches/num_total:.2%})')