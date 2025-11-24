import pathlib 
import os 
import subprocess
import threading
import time
import signal
import sys
from queue import Queue
import colorama 
import argparse 
import json

parser = argparse.ArgumentParser(description="Run selected algorithm tests.")
parser.add_argument(
    "--alg",
    type=str,
    help="Algorithm to run (alg23, alg24, bracha, imbsraynal)",
)
parser.add_argument(
    "--list",
    type=str,
    help="List of tests to run (JSON array)",
)
args = parser.parse_args()

RED = colorama.Fore.RED
RESET = colorama.Fore.RESET

SIMULATION_FILE = pathlib.Path(__file__).parent / "quantas" / "Common" / "Simulation.hpp"
MAKEFILE_FILE = pathlib.Path(__file__).parent / f"makefile_{args.alg}"
STATUS_FILE = pathlib.Path(__file__).parent / f"status_{args.alg}.txt"
MAX_CONCURRENCY = 4

# --- 1) Empty the status file at start ---
#STATUS_FILE.write_text("")  # truncate/clear

start_time = time.time()


ALGORITHMS = [
    ("BrachaPeer", "bracha.json"),
    ("Alg23Peer", "alg23.json"),
    ("Alg24Peer", "alg24.json"),
    ("ImbsRaynalPeer", "imbsraynal.json"),
    ("COOLPeer", "cool.json")
]

ALGORITHMS_TO_RUN = []
def get_tests():
    global ALGORITHMS_TO_RUN
    ALGORITHMS_TO_RUN.clear()
    alg_filter = args.alg
    directory_alg23 = pathlib.Path(__file__).parent / "quantas" / "Alg23Peer"
    directory_alg24 = pathlib.Path(__file__).parent / "quantas" / "Alg24Peer"
    directory_bracha = pathlib.Path(__file__).parent / "quantas" / "BrachaPeer"
    directory_imbsraynal = pathlib.Path(__file__).parent / "quantas" / "ImbsRaynalPeer"
    directory_cool = pathlib.Path(__file__).parent / "quantas" / "COOLPeer"
    dirs = []
    if alg_filter == "alg23":
        dirs = [(directory_alg23, "Alg23Peer")]
    elif alg_filter == "alg24":
        dirs = [(directory_alg24, "Alg24Peer")]
    elif alg_filter == "bracha":
        dirs = [(directory_bracha, "BrachaPeer")]
    elif alg_filter == "imbsraynal":
        dirs = [(directory_imbsraynal, "ImbsRaynalPeer")]
    elif alg_filter == "cool":
        dirs = [(directory_cool, "COOLPeer")]

    for directory, alg_class in dirs:
        json_files = sorted([f for f in os.listdir(directory) if f.endswith(".json") and "test" not in f])
        for json_file in json_files:
            ALGORITHMS_TO_RUN.append((alg_class, json_file))
    
    print(f"Found {len(ALGORITHMS_TO_RUN)} tests to run for algorithm filter '{alg_filter}'.")


if args.list is None:
    get_tests()
else:
    ALGORITHMS_TO_RUN = json.loads(args.list)
    print(f"Running {len(ALGORITHMS_TO_RUN)} tests from provided list for algorithm '{args.alg}'.")
    json_str = json.dumps(ALGORITHMS_TO_RUN)
    print(json_str)



# Global stop event and process list
stop_event = threading.Event()
processes = []
processes_lock = threading.Lock()

def change_makefile(alg_peer, alg_json):
    with open(MAKEFILE_FILE, "r") as f:
        lines = f.readlines()
    
    text = ""
    do_nothing = False
    for line in lines:
        
        if "#----->" in line:
            do_nothing = True
            text += "#----->\n"
            text += f"INPUTFILE := {alg_json}\n"
            text += f"ALGFILE := {alg_peer}\n"
            text += "#<-----\n"
        elif "#<-----" in line:
            do_nothing = False
        elif do_nothing:
            continue 
        else:
            text += line
            
    with open(MAKEFILE_FILE, "w") as f:
        f.write(text)


def run_test(alg_peer, alg_json):
    change_makefile(alg_peer, alg_json)
    print(RED,f"Running tests for {alg_json}",RESET)
    cmd = ["make", "-f", str(MAKEFILE_FILE), "run"]
    
    # Start the subprocess (non-blocking)
    proc = subprocess.Popen(cmd)
    with processes_lock:
        processes.append(proc)

    # Wait for it to complete or for stop_event
    while proc.poll() is None:
        if stop_event.is_set():
            print(f"[runner] Terminating {alg_peer}...", flush=True)
            proc.terminate()
            try:
                proc.wait(timeout=5)
            except subprocess.TimeoutExpired:
                proc.kill()
            break
        time.sleep(0.5)

    with processes_lock:
        if proc in processes:
            processes.remove(proc)

    if proc.returncode == 0:
        print(f"[runner] done: {' '.join(cmd)}", flush=True)
    elif not stop_event.is_set():
        print(f"[runner] ERROR: {' '.join(cmd)} exited with {proc.returncode}", flush=True)



def handle_sigint(signum, frame):
    print("\n[!] Caught Ctrl-C, stopping all threads and subprocesses...", flush=True)
    stop_event.set()

    # Terminate all running processes
    with processes_lock:
        for proc in processes:
            if proc.poll() is None:
                proc.terminate()
        for proc in processes:
            try:
                proc.wait(timeout=3)
            except subprocess.TimeoutExpired:
                proc.kill()

    sys.exit(1)
    
    
def worker(q: Queue):
    while not stop_event.is_set():
        try:
            alg_peer, alg_json = q.get_nowait()
        except Exception:
            break  # queue is empty
        try:
            run_test(alg_peer, alg_json)
        finally:
            q.task_done()
    


def main():
    signal.signal(signal.SIGINT, handle_sigint)

    # Fill the queue with all algorithms to run
    q = Queue()
    for alg_peer, alg_json in ALGORITHMS_TO_RUN:
        q.put((alg_peer, alg_json))

    # Start up to MAX_CONCURRENCY worker threads
    threads = [
        threading.Thread(target=worker, args=(q,), daemon=True)
        for _ in range(min(MAX_CONCURRENCY, q.qsize()))
    ]

    try:
        for t in threads:
            t.start()
            time.sleep(30)

        # Wait for all tasks to finish
        while not q.empty():
            time.sleep(2)  # keep the main thread alive and responsive to SIGINT

        q.join()  # all done!

    finally:
        stop_event.set()
        for t in threads:
            t.join()


if __name__ == "__main__":
    main()
    end_time = time.time()
    elapsed_time = end_time - start_time
    print(f"All tests completed in {elapsed_time:.2f} seconds.")



