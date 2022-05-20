#!/usr/bin/env python3

import docker
import argparse
import math
import sys
import os
import h5py
import datetime
import socket
import pandas as pd

MAX_LOG_N = 30
DEFAULT_CPU = 0
DEFAULT_MAX_MEMORY = 128*2**30
NS = [(1 << i//2) if i%2 == 0 else int(round(math.sqrt(2)*(1 << i//2))) \
          for i in range(8,2*MAX_LOG_N+1)]
MS = [1 << i for i in range(4,19)]
NUM_REPS = 10
DATATYPES = ['uint64', 'str', 'jr']
ALGORITHMS = ['hyperloglog', 'hyperloglogzstd', 'hyperlogloglog',
                  'hyperloglogloga', # append only
                  'hyperlogloglogi', # increase only
                  'hyperlogloglogai', # append and increase only
                  'hyperlogloglogb', # bottom variant
                  'hashonly', 
                  'apache-hll4', # apache datasketches hyperloglog with 4 bits
                  'apache-hll6', # apache datasketches hyperloglog with 6 bits
                  'apache-hll8', # apache datasketches hyperloglog with 8 bits
                  'apache-cpc', # apache datasketches compressed probability counting
                  'zetasketch' # google zetasketch
                  ]
MODES = ['query', 'merge']
RANDOM_STRING_LENGTH = 8
INITIAL_SEED = 0x11e3ea10

def instance_filename_stub(instance):
    return f"results/{instance['mode']}_{instance['algo']}_" + \
      f"{instance['dt']}_{instance['m']}_{instance['n']}"

def populate_seeds(initial_seed):
    global RANDOM_NUMBER_SEEDS
    RANDOM_NUMBER_SEEDS = dict()
    seed = initial_seed
    for ds in DATATYPES:
        RANDOM_NUMBER_SEEDS[ds] = dict()
        for m in MS:
            RANDOM_NUMBER_SEEDS[ds][m] = dict()
            for n in NS:
                RANDOM_NUMBER_SEEDS[ds][m][n] = dict()
                for rep in range(NUM_REPS):
                    RANDOM_NUMBER_SEEDS[ds][m][n][rep] = seed
                    seed += 1

                    

def filter_instances(mode_filter, algo_filter, dt_filter, m_filter,
                         n_filter):
    instances = list()
    for mode in MODES:
        if mode_filter is not None and mode != mode_filter:
            continue
        for algo in ALGORITHMS:
            if algo_filter is not None and algo != algo_filter:
                continue
            if mode == 'merge' and algo == 'hashonly':
                continue
            for dt in DATATYPES:
                if dt_filter is not None and dt != dt_filter:
                    continue
                if not algo.startswith('hyperloglog') and dt == 'jr':
                    continue
                for m in MS:
                    if m_filter is not None and m_filter != m:
                        continue
                    if algo == 'zetasketch' and m < 1024:
                        continue
                    if algo == 'apache-cpc' and m >= 524288:
                        continue
                    for n in NS:
                        if n_filter is not None and n_filter != n:
                            continue
                        instance = {
                            'mode' : mode,
                            'algo' : algo,
                            'dt' : dt,
                            'm' : m,
                            'n' : n
                        }
                        instances.append(instance)
    return instances



def construct_docker_command(instance, seed):
    mode = instance['mode']
    algo = instance['algo']
    dt = instance['dt']
    m = instance['m']
    n = instance['n']
    cmd = f'/bin/bash -c \'inputgenerator/inputgenerator '
    if dt == 'str':
        cmd += f'--len {RANDOM_STRING_LENGTH} '
    elif dt == 'jr':
        cmd += f'-m {m} '
    cmd += f'{n} {dt} {seed} | '
    if algo.startswith('hyperloglog') or algo == 'hashonly':
        cmd += 'hyperlogloglog/measure '
        if algo == 'hyperlogloglog':
            cmd += '--flags default '
        elif algo == 'hyperloglogloga':
            cmd += '--flags appendonly '
        elif algo == 'hyperlogloglogi':
            cmd += '--flags increaseonly '
        elif algo == 'hyperlogloglogai':
            cmd += '--flags appendincreaseonly '
        elif algo == 'hyperlogloglogb':
            cmd += '--flags bottom '
    if algo.startswith('apache-'):
        cmd += 'datasketches/measure '
        if algo == 'apache-hll4':
            cmd += '--hll-bits 4 '
        elif algo == 'apache-hll6':
            cmd += '--hll-bits 6 '
        elif algo == 'apache-hll8':
            cmd += '--hll-bits 8 '
    if algo == 'zetasketch':
        cmd += 'java -Xms96g -Xmx96g -jar zetasketch/measure.jar '
    if (algo.startswith('hyperloglog') or algo == 'hashonly' or algo.startswith('apache')) and dt == 'str':
        cmd += f'--len {RANDOM_STRING_LENGTH} '
    cmd += f'{mode} '
    if algo in ['hyperloglog', 'hyperloglogzstd', 'hyperlogloglog', 'hashonly']:
        cmd += f'{algo} '
    elif algo.startswith('hyperlogloglog'):
        cmd += 'hyperlogloglog '
    elif algo.startswith('apache-hll'):
        cmd += 'hll '
    elif algo == 'apache-cpc':
        cmd += 'cpc '
    cmd += f'{dt} {m} {n}'
    if algo == 'zetasketch' and dt == 'str':
        cmd += f' {RANDOM_STRING_LENGTH}'
    cmd += '\''
    return cmd

def run(instance, container_name, cpu, max_memory):
    mode = instance['mode']
    algo = instance['algo']
    dt = instance['dt']
    m = instance['m']
    n = instance['n']
    sys.stderr.write(f"running {mode} {algo} {dt} {m} {n}\n")
    filename_stub = instance_filename_stub(instance) 
    hdf5_filename = filename_stub + '.hdf5'
    stdout_filename = filename_stub + '.stdout'
    stderr_filename = filename_stub + '.stderr'
    log_filename = filename_stub + '.log'

    client = docker.from_env()
    logf = open(log_filename,'w')
    stdoutf = open(stdout_filename, 'w')
    stderrf = open(stderr_filename, 'w')
    results = list()
    
    for rep in range(NUM_REPS):
        seed = RANDOM_NUMBER_SEEDS[dt][m][n][rep]
        cmd = construct_docker_command(instance, seed)
        logf.write(f'start {datetime.datetime.now()}\n' +
                       f'rep {rep}\n' +
                       f'container {container_name}\n' +
                       f'cmd {cmd}\n' +
                       f'hostname {socket.gethostname()}\n')
        
        container = client.containers.run(container_name, cmd, detach = True,
                                              cpuset_cpus = str(cpu),
                                              mem_limit = str(max_memory),
                                              mem_swappiness = 0,
                                              memswap_limit = str(max_memory))
        result = container.wait()
        logf.write(f'Error: {result["Error"]}\n')
        logf.write(f'StatusCode: {result["StatusCode"]}\n')

        stdout = container.logs(stdout = True, stderr = False).decode('utf-8')
        stderr = container.logs(stdout = False, stderr = True).decode('utf-8')

        stdoutf.write(stdout)
        stderrf.write(stderr)

        logf.write(f'end {datetime.datetime.now()}\n')

        result = dict(map(lambda x: (x[0], float(x[1])), [line.split(' ') for line in stdout.strip().split('\n')]))
        results.append(result)

    logf.close()
    stderrf.close()
    stdoutf.close()

    df = pd.DataFrame(results, columns = ['time', 'estimate', 'bitsize', 'compressCount', 'rebaseCount'] )
    with h5py.File(hdf5_filename, 'w') as f:
        f.create_dataset('measurements', data=df.to_numpy())
        f.attrs['mode'] = mode
        f.attrs['algo'] = algo
        f.attrs['dt'] = dt
        f.attrs['m'] = m
        f.attrs['n'] = n

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--container-name', type=str, default='hyperlogloglog',
                            help = 'name of the container to run the experiments in')
    parser.add_argument('--cpu', type=int, default=DEFAULT_CPU,
                            help = 'which cpu to use')
    parser.add_argument('--max-memory', type=int, default = DEFAULT_MAX_MEMORY,
                            help = 'maximum amount of memory to use (in bytes)')
    parser.add_argument('--random-seed', type = int, default = INITIAL_SEED,
                            help = 'random number generator seed')
    parser.add_argument('--algorithm', type = str, choices = ALGORITHMS,
                            help = 'if set, only run the specified algorithm')
    parser.add_argument('-n', type = int, choices = NS,
                            help = 'if set, only run the specified n')
    parser.add_argument('-m', type = int, choices = MS,
                            help = 'if set, only run with the specified number of registers')
    parser.add_argument('--mode', type = str, choices = MODES,
                            help = 'if set, only run with the specified mode')
    parser.add_argument('--datatype', type = str, choices = DATATYPES,
                            help = 'if set, only run with the specified datatype')
    args = parser.parse_args()

    sys.stderr.write('populating random number seeds... ')
    populate_seeds(args.random_seed)
    sys.stderr.write('done\n')

    sys.stderr.write('filtering instances... ')
    instances = filter_instances(args.mode,args.algorithm,args.datatype,args.m,args.n)
    sys.stderr.write('done\n')
    sys.stderr.write('{} instances\n'.format(len(instances)))

    if not os.path.exists('results/'):
        sys.stderr.write("creating `results/'... ")
        os.mkdir('results/')
        sys.stderr.write('done\n')

    sys.stderr.write('filtering out instances that have already been run... ')
    instances = list(filter(lambda i: not os.path.exists(instance_filename_stub(i) + '.hdf5'), instances))
    sys.stderr.write('done\n')
    sys.stderr.write('{} instances to run\n'.format(len(instances)))

    for instance in instances:
        run(instance, args.container_name, args.cpu, args.max_memory)

if __name__ == '__main__':
    main()
