import pathlib 
import matplotlib.pyplot as plt
import numpy as np
import json
from scipy import stats
from utils import *
import math


def plots_single_algorithm(alg):
    results = get_all_results(alg)
    for combination in results:
        comb = results[combination]
        
        # Symmetric, tiny horizontal offsets
        offsets = np.linspace(-0.9, 0.9, len(F_VALUES))

        fig, axes = plt.subplots(2, 3, figsize=(14, 10), sharex=True)
        axes = axes.ravel()
            
        for idx, info in enumerate(INFOS):
            ax = axes[idx]
            for i, f in enumerate(F_VALUES):
                x = [p + offsets[i] for p in P_VALUES]
                y = [comb[f][p][info][0] for p in P_VALUES]
                # Plot the mean line and keep handle for color
                ln, = ax.plot(
                    x, y,
                    marker='o',
                    linewidth=2,
                    markerfacecolor='white',
                    markeredgewidth=1.5,
                    label=f'{f} byzantine nodes'
                )

                # Confidence interval shading (min/max)
                y_min = [comb[f][p][info][1] for p in P_VALUES]
                y_max = [comb[f][p][info][2] for p in P_VALUES]
                ax.fill_between(x, y_min, y_max, alpha=0.15, color=ln.get_color(), linewidth=0)
            ax.set_title(f"{info}")
            ax.grid(True, linestyle=':', alpha=0.4)
            ax.set_xlabel("message diversity (p)")
                    
                    
        handles, labels = axes[0].get_legend_handles_labels()
        fig.legend(handles, labels, loc='lower center', ncol=len(F_VALUES), frameon=False)
        fig.suptitle(f"{alg} {combination}", y=0.98)
        fig.tight_layout(rect=[0, 0.08, 1, 0.95])
        #plt.show()
        save_path = DIR_IMG / alg / f"{alg}_{combination}.png"
        save_path.parent.mkdir(parents=True, exist_ok=True)
        plt.savefig(save_path, bbox_inches='tight', dpi=900)
        plt.close(fig)
        

THRESHOLDS = {"alg23": 20, "imbsraynal": 19, "bracha": 33, "alg24": 25, "cool": 33}
ALGORITHM_COMPARISON_LIVENESS = [("alg23", "silent"), ("imbsraynal", "silent"), ("bracha", "silent_silent"), ("alg24", "opposite_silent_silent"), ("cool", "silent_silent_silent_silent_silent_silent")]
INFOS_LIVENESS = "delivery_nodes"
ALGORITHM_COMPARISON_CORRECTNESS = [("alg23", "opposite"), ("imbsraynal", "opposite"), ("bracha", "silent_silent"), ("alg24", "opposite_opposite_opposite"), ("cool", "opposite_send_send_send_opposite_opposite")]
INFOS_CORRECTNESS = "disagreement"
INFOS_CORRECTNESS_FREQUENCY = "disagreement_frequency"
def plot_all_algorithms(algorithms, info):
    d = {}
    for alg, combination in algorithms:
        d[alg] = {}
        d[alg][combination] = {}
    for alg, combination in algorithms:
        res_alg = get_all_results(alg)
        d[alg][combination] = res_alg[combination]
    
    fig, axes = plt.subplots(2, 3, figsize=(14, 10), sharex=True)
    axes = axes.ravel()
    offsets_alg = np.linspace(-0.9, 0.9, len(algorithms))
    
    for idx_alg, alg in enumerate(d):
        for combination in d[alg]:
            comb = d[alg][combination]
            
            for idx, f in enumerate(F_VALUES):
                ax = axes[idx]
                x = [p + offsets_alg[idx_alg] for p in P_VALUES]
                y = [comb[f][p][info][0] for p in P_VALUES]
                # Plot the mean line and keep handle for color
                ln, = ax.plot(
                    x, y,
                    marker='o',
                    linewidth=2,
                    markerfacecolor='white',
                    markeredgewidth=1.5,
                    label=f'{alg}_{combination}  (t≤{THRESHOLDS[alg]})'
                )

                # Confidence interval shading (min/max)
                y_min = [comb[f][p][info][1] for p in P_VALUES]
                y_max = [comb[f][p][info][2] for p in P_VALUES]
                ax.fill_between(x, y_min, y_max, alpha=0.15, color=ln.get_color(), linewidth=0)
                info_title = info
                if info == "delivery_nodes":
                    info_title = "terminating nodes"
                ax.set_title(f"{info_title} with f={f}")
                ax.grid(True, linestyle=':', alpha=0.4)
                ax.set_xlabel("message diversity (p)")
                        
                        
    handles, labels = axes[0].get_legend_handles_labels()
    ncol = math.ceil(len(algorithms) / 2)
    fig.legend(handles, labels, loc='lower center', ncol=ncol, frameon=False, fontsize=14)
    fig.suptitle(f"comparison_{info}", y=0.98, fontsize=20)
    fig.tight_layout(rect=[0, 0.08, 1, 1])
    #plt.show()
    plt.savefig(DIR_IMG / f"comparison_{info}.png", bbox_inches='tight', dpi=900)
    plt.close(fig)



def plot_all_algorithms_big(algorithms, info1, info2, info3):
    d = {}
    for alg, combination in algorithms:
        d[alg] = {}
        d[alg][combination] = {}
    for alg, combination in algorithms:
        res_alg = get_all_results(alg)
        d[alg][combination] = res_alg[combination]
    
    #fig, axes = plt.subplots(3, 5, figsize=(24, 12), sharex=True)
    fig, axes = plt.subplots(3, 5, figsize=(24, 12))
    axes = axes.ravel()
    for ax in axes:
        ax.set_xticks(P_VALUES)
        ax.set_xticklabels([str(p) for p in P_VALUES], fontsize=16)
        ax.tick_params(axis='both', which='major', labelsize=16)
    offsets_alg = np.linspace(-0.9, 0.9, len(algorithms))
    
    for idx_alg, alg in enumerate(d):
        for combination in d[alg]:
            comb = d[alg][combination]
            
            for idx, f in enumerate(F_VALUES):
                ax = axes[idx]
                x = [p + offsets_alg[idx_alg] for p in P_VALUES]
                y = [comb[f][p][info1][0] for p in P_VALUES]
                # Plot the mean line and keep handle for color
                ln, = ax.plot(
                    x, y,
                    marker='o',
                    linewidth=2,
                    markerfacecolor='white',
                    markeredgewidth=1.5,
                    label=f'{alg}_{combination}  (t≤{THRESHOLDS[alg]})'
                )

                # Confidence interval shading (min/max)
                y_min = [comb[f][p][info1][1] for p in P_VALUES]
                y_max = [comb[f][p][info1][2] for p in P_VALUES]
                ax.fill_between(x, y_min, y_max, alpha=0.15, color=ln.get_color(), linewidth=0)
                info1_title = info1
                if info1 == "delivery_nodes":
                    info1_title = "terminating nodes"
                ax.set_title(f"{info1_title} with f={f}", fontsize=16)
                ax.grid(True, linestyle=':', alpha=0.4)
                ax.set_xlabel("equivocation diversity", fontsize=16)
            
            for idx, f in enumerate(F_VALUES):
                ax = axes[idx+5]
                x = [p + offsets_alg[idx_alg] for p in P_VALUES]
                y = [comb[f][p][info2][0] for p in P_VALUES]
                # Plot the mean line and keep handle for color
                ln, = ax.plot(
                    x, y,
                    marker='o',
                    linewidth=2,
                    markerfacecolor='white',
                    markeredgewidth=1.5,
                    label=f'{alg}_{combination}  (t≤{THRESHOLDS[alg]})'
                )

                # Confidence interval shading (min/max)
                y_min = [comb[f][p][info2][1] for p in P_VALUES]
                y_max = [comb[f][p][info2][2] for p in P_VALUES]
                ax.fill_between(x, y_min, y_max, alpha=0.15, color=ln.get_color(), linewidth=0)
                ax.set_title(f"{info2} with f={f}", fontsize=16)
                ax.grid(True, linestyle=':', alpha=0.4)
                ax.set_xlabel("equivocation diversity", fontsize=16)
            
            for idx, f in enumerate(F_VALUES):
                ax = axes[idx+10]
                x = [p + offsets_alg[idx_alg] for p in P_VALUES]
                y = [comb[f][p][info3][0] for p in P_VALUES]
                # Plot the mean line and keep handle for color
                ln, = ax.plot(
                    x, y,
                    marker='o',
                    linewidth=2,
                    markerfacecolor='white',
                    markeredgewidth=1.5,
                    label=f'{alg}_{combination}  (f<={THRESHOLDS[alg]})'
                )

                # Confidence interval shading (min/max)
                y_min = [comb[f][p][info3][1] for p in P_VALUES]
                y_max = [comb[f][p][info3][2] for p in P_VALUES]
                ax.fill_between(x, y_min, y_max, alpha=0.15, color=ln.get_color(), linewidth=0)
                ax.set_title(f"{info3} with f={f}", fontsize=16)
                ax.grid(True, linestyle=':', alpha=0.4)
                ax.set_xlabel("equivocation diversity", fontsize=16)
                        
                        
    handles, labels = axes[0].get_legend_handles_labels()
    ncol = math.ceil(len(algorithms) / 2)
    fig.legend(handles, labels, loc='lower center', ncol=ncol, frameon=False, fontsize=18)
    #fig.suptitle(f"comparison_{info1}_{info2}_{info3}", y=0.98, fontsize=20)
    fig.tight_layout(rect=[0, 0.08, 1, 1])
    #plt.show()
    plt.savefig(DIR_IMG / f"comparison_{info1}_{info2}_{info3}.png", bbox_inches='tight', dpi=900)
    plt.close(fig)


def plot_all_algorithms_big_vertical(algorithms, info1, info2, info3):
    d = {}
    for alg, combination in algorithms:
        d[alg] = {}
        d[alg][combination] = {}
    for alg, combination in algorithms:
        res_alg = get_all_results(alg)
        d[alg][combination] = res_alg[combination]
    
    #fig, axes = plt.subplots(3, 5, figsize=(24, 12), sharex=True)
    fig, axes = plt.subplots(5, 3, figsize=(16, 24))
    axes = axes.ravel()
    for ax in axes:
        ax.set_xticks(P_VALUES)
        ax.set_xticklabels([str(p) for p in P_VALUES], fontsize=16)
        ax.tick_params(axis='both', which='major', labelsize=16)
    offsets_alg = np.linspace(-0.9, 0.9, len(algorithms))
    
    for idx_alg, alg in enumerate(d):
        for combination in d[alg]:
            comb = d[alg][combination]
            
            for idx, f in enumerate(F_VALUES):
                ax = axes[idx*3]
                x = [p + offsets_alg[idx_alg] for p in P_VALUES]
                y = [comb[f][p][info1][0] for p in P_VALUES]
                # Plot the mean line and keep handle for color
                ln, = ax.plot(
                    x, y,
                    marker='o',
                    linewidth=2,
                    markerfacecolor='white',
                    markeredgewidth=1.5,
                    label=f'{alg}_{combination}  (t≤{THRESHOLDS[alg]})'
                )

                # Confidence interval shading (min/max)
                y_min = [comb[f][p][info1][1] for p in P_VALUES]
                y_max = [comb[f][p][info1][2] for p in P_VALUES]
                ax.fill_between(x, y_min, y_max, alpha=0.15, color=ln.get_color(), linewidth=0)
                info1_title = info1
                if info1 == "delivery_nodes":
                    info1_title = "terminating nodes"
                ax.set_title(f"{info1_title} with f={f}", fontsize=16)
                ax.grid(True, linestyle=':', alpha=0.4)
                ax.set_xlabel("equivocation diversity", fontsize=16)
            
            for idx, f in enumerate(F_VALUES):
                ax = axes[idx*3+1]
                x = [p + offsets_alg[idx_alg] for p in P_VALUES]
                y = [comb[f][p][info2][0] for p in P_VALUES]
                # Plot the mean line and keep handle for color
                ln, = ax.plot(
                    x, y,
                    marker='o',
                    linewidth=2,
                    markerfacecolor='white',
                    markeredgewidth=1.5,
                    label=f'{alg}_{combination}  (t≤{THRESHOLDS[alg]})'
                )

                # Confidence interval shading (min/max)
                y_min = [comb[f][p][info2][1] for p in P_VALUES]
                y_max = [comb[f][p][info2][2] for p in P_VALUES]
                ax.fill_between(x, y_min, y_max, alpha=0.15, color=ln.get_color(), linewidth=0)
                ax.set_title(f"{info2} with f={f}", fontsize=16)
                ax.grid(True, linestyle=':', alpha=0.4)
                ax.set_xlabel("equivocation diversity", fontsize=16)
            
            for idx, f in enumerate(F_VALUES):
                ax = axes[idx*3+2]
                x = [p + offsets_alg[idx_alg] for p in P_VALUES]
                y = [comb[f][p][info3][0] for p in P_VALUES]
                # Plot the mean line and keep handle for color
                ln, = ax.plot(
                    x, y,
                    marker='o',
                    linewidth=2,
                    markerfacecolor='white',
                    markeredgewidth=1.5,
                    label=f'{alg}_{combination}  (f<={THRESHOLDS[alg]})'
                )

                # Confidence interval shading (min/max)
                y_min = [comb[f][p][info3][1] for p in P_VALUES]
                y_max = [comb[f][p][info3][2] for p in P_VALUES]
                ax.fill_between(x, y_min, y_max, alpha=0.15, color=ln.get_color(), linewidth=0)
                ax.set_title(f"{info3} with f={f}", fontsize=16)
                ax.grid(True, linestyle=':', alpha=0.4)
                ax.set_xlabel("equivocation diversity", fontsize=16)
                        
                        
    handles, labels = axes[0].get_legend_handles_labels()
    ncol = math.ceil(2)
    fig.legend(handles, labels, loc='lower center', ncol=ncol, frameon=False, fontsize=18)
    #fig.suptitle(f"comparison_{info1}_{info2}_{info3}", y=0.98, fontsize=20)
    fig.tight_layout(rect=[0, 0.08, 1, 1])
    #plt.show()
    plt.savefig(DIR_IMG / f"comparison_vertical_{info1}_{info2}_{info3}.png", bbox_inches='tight', dpi=900)
    plt.close(fig)

            

"""
plots_single_algorithm("alg23")     
plots_single_algorithm("imbsraynal")   
plots_single_algorithm("bracha")   
plots_single_algorithm("alg24")
plots_single_algorithm("cool")
"""


""" 
plot_all_algorithms(ALGORITHM_COMPARISON_LIVENESS, INFOS_LIVENESS)
plot_all_algorithms(ALGORITHM_COMPARISON_CORRECTNESS, INFOS_CORRECTNESS)
plot_all_algorithms(ALGORITHM_COMPARISON_CORRECTNESS, INFOS_CORRECTNESS_FREQUENCY)
"""


#plot_all_algorithms_big(ALGORITHM_COMPARISON_CORRECTNESS,INFOS_LIVENESS, INFOS_CORRECTNESS, INFOS_CORRECTNESS_FREQUENCY)
plot_all_algorithms_big_vertical(ALGORITHM_COMPARISON_CORRECTNESS,INFOS_LIVENESS, INFOS_CORRECTNESS, INFOS_CORRECTNESS_FREQUENCY)



