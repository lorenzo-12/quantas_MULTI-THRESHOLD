import pathlib 
import os 
import subprocess
import threading
import time
import signal
import sys
import json

TOTAL_TESTS = 1500
ALL_TESTS = []
def get_all_tests():
    global ALL_TESTS
    directory_alg23 = pathlib.Path(__file__).parent.parent / "quantas" / "Alg23Peer"
    directory_alg24 = pathlib.Path(__file__).parent.parent / "quantas" / "Alg24Peer"
    directory_bracha = pathlib.Path(__file__).parent.parent / "quantas" / "BrachaPeer"
    directory_imbsraynal = pathlib.Path(__file__).parent.parent / "quantas" / "ImbsRaynalPeer"
    directory_cool = pathlib.Path(__file__).parent.parent / "quantas" / "COOLPeer"
    dirs = [ directory_alg23, directory_alg24, directory_bracha, directory_imbsraynal, directory_cool ]
    for directory in dirs:
        l = [ file_name for file_name in os.listdir(directory) if file_name.endswith(".json") and "test" not in file_name]
        ALL_TESTS.extend(l)


status_file = pathlib.Path(__file__).parent.parent / "status_<alg>.txt"
def reader_status():
    d = {}
    for f in ALL_TESTS:
        d[f] = 0
    
    all_lines = []
    with open(str(status_file).replace("<alg>", "bracha"), "r") as f:
        lines = f.readlines()
        all_lines += lines
    with open(str(status_file).replace("<alg>", "imbsraynal"), "r") as f:
        lines = f.readlines()
        all_lines += lines
    with open(str(status_file).replace("<alg>", "alg23"), "r") as f:
        lines = f.readlines()
        all_lines += lines
    with open(str(status_file).replace("<alg>", "alg24"), "r") as f:
        lines = f.readlines()
        all_lines += lines
    with open(str(status_file).replace("<alg>", "cool"), "r") as f:
        lines = f.readlines()
        all_lines += lines
    for line in all_lines:
        line = line.strip().replace('"',"")
        if line in ALL_TESTS:
            d[line] +=1

    
    tmp = ""
    for test_name in sorted(d):
        if ("alg23" in test_name):
            count = d[test_name]
            perc = min(int((count/TOTAL_TESTS)*100), 100)
            test_name = test_name.replace(".json","")
            tmp += f"{test_name:<40}: [{'#'*perc}{'.'*(100-perc)}] {perc:<3}%  - {count:>4}/{TOTAL_TESTS}\n"
    tmp += "\n"
    
    for test_name in sorted(d):
        if ("alg24" in test_name):
            count = d[test_name]
            perc = min(int((count/TOTAL_TESTS)*100), 100)
            test_name = test_name.replace(".json","")
            tmp += f"{test_name:<40}: [{'#'*perc}{'.'*(100-perc)}] {perc:<3}%  - {count:>4}/{TOTAL_TESTS}\n"
    tmp += "\n"
    
    for test_name in sorted(d):
        if ("bracha" in test_name):
            count = d[test_name]
            perc = min(int((count/TOTAL_TESTS)*100), 100)
            test_name = test_name.replace(".json","")
            tmp += f"{test_name:<40}: [{'#'*perc}{'.'*(100-perc)}] {perc:<3}%  - {count:>4}/{TOTAL_TESTS}\n"
    tmp += "\n"
    
    for test_name in sorted(d):
        if ("imbsraynal" in test_name):
            count = d[test_name]
            perc = min(int((count/TOTAL_TESTS)*100), 100)
            test_name = test_name.replace(".json","")
            tmp += f"{test_name:<40}: [{'#'*perc}{'.'*(100-perc)}] {perc:<3}%  - {count:>4}/{TOTAL_TESTS}\n"
    
    tmp += "\n"
    
    for test_name in sorted(d):
        if ("cool" in test_name):
            count = d[test_name]
            perc = min(int((count/TOTAL_TESTS)*100), 100)
            test_name = test_name.replace(".json","")
            if (len(test_name.strip())>40):
                test_name = test_name.replace("silent","sil").replace("opposite","opp")
            tmp += f"{test_name:<40}: [{'#'*perc}{'.'*(100-perc)}] {perc:<3}%  - {count:>4}/{TOTAL_TESTS}\n"
    

    os.system('clear')
    print("Status so far:")
    print(tmp)
    

get_all_tests()
while True:
    reader_status()
    time.sleep(5)

