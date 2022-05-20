#!/usr/bin/env python3

import h5py
import os
import pandas as pd
import numpy as np
import sys

def main():
    results = list()
    for root, dirs, files in os.walk('results/'):
        for fn in files:
            if not fn.endswith('.hdf5'):
                continue
            filename = root + '/' + fn
            with h5py.File(filename,'r') as f:
                data = np.array(f['measurements'])
                num_reps = data.shape[0]
                for rep in range(num_reps):
                    result = { **dict(f.attrs), **{
                        'rep' : rep,
                        'time' : data[rep,0],
                        'estimate' : data[rep,1],
                        'bitsize' : data[rep,2],
                        'compressCount' : data[rep,3],
                        'rebaseCount' : data[rep,4]
                    }}
                    results.append(result)
    pd.DataFrame(results).to_csv(sys.stdout,index = False)

    

if __name__ == '__main__':
    main()
