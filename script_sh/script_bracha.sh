#!/bin/bash

#OAR -l { host in ('big16','big17','big18', 'big19', 'big20')}/nodes=1/core=48,walltime=1000:0:0

source /etc/profile.d/modules.sh		# Shell initialization to use module
module purge							# Environment cleanup
module load python/anaconda3			# Loading of anaconda 3 module

make clean
python run_tests_groups.py --alg "bracha"
