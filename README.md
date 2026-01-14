# QUANTAS
A Quantitative User-friendly Adaptive Networked Things Abstract Simulator.

This project is a simulator that enables quantitative performance analysis of distributed algorithms. QUANTAS is an abstract simulator, therefore, the obtained results are not affected by the specifics of a particular network or operating system architecture. QUANTAS allows distributed algorithms researchers to quickly investigate a potential solution  and collect data about its performance. QUANTAS programming is relatively straightforward and is accessible to theoretical researchers. 


---


## FULL PAPER
This anonymous repository contains the full version of the paper submitted to **ICDCS 2026**, including:
- full proofs for all theorems
- complete experimental results
- all figures/plots from the evaluation

Paper file:
`ICDCS__Multi_Threshold_BRB___FULL_PAPER___anonymous_4open_science_.pdf`


---


## Repository Structure

### `quantas/`
Core code used to run the experiments. It contains multiple algorithm implementations, including:

- `Alg23Peer/`
- `Alg24Peer/`
- `BrachaPeer/`
- `COOLPeer/`
- `ImbsRaynalPeer/`

Each algorithm directory contains:
- the **topologies** used in experiments
- the `.cpp` implementation that replicates the corresponding algorithm


### `results_all/`
Experimental outputs in JSON format for **all test combinations**.

Results are organized:
1. by algorithm
2. then by the **strategy used by message adversaries**


### `img/`
Plots generated from the JSON files in `results_all/`.

This directory includes:
- per-algorithm performance plots
- comparative plots across algorithms


### `script_python/`
Python utilities used during experimentation, including:

- `generate_json.py` — generate random topologies
- `evaluation.py` — compute metrics and generate plots (saved to `img/`)
- `check_status.py` — check execution status of test batches


### `script_sh/`
Shell scripts used to run experiments on the university servers.


### `HOWTO.md`
This file contains a step-by-step how-to to write and simulate a distributed algorithm with QUANTAS. 


### `run_tests_groups.py`
Main Python entry point used to launch experiments (referenced by the scripts in `script_sh/`).

It:
- takes as input the configuration of the desired tests
- runs them using Python threads to speed up execution


## How to Replicate the Experiments

To replicate the experiments in this repository (Linux):

1. **Download and extract** this repository on your machine.
2. Open a terminal and **cd into** the repository root directory.
3. Ensure all **dependencies** are installed and correctly configured.
4. Follow the instructions in **`HOWTO.md`** to verify that the simulator builds and runs correctly (i.e., the basic workflow works end-to-end).
5. Run:

```bash
make clean
python run_tests_groups.py --alg "<algorithm_name>"
```

where `<algorithm_name>` can be one of `[imbsraynal, cool, bracha, alg24, alg23]`.

Tip: the shell scripts in script_sh/ show the exact logic/configurations used to launch batches of experiments on the university servers.


## Notes
- This is an **anonymous** research repository for open science / review.
