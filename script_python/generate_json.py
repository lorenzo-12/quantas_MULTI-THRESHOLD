import pathlib 
import os
import json 
import random


DIR_FILE_JSON = pathlib.Path(__file__).parent.parent / "quantas"
RESULTS_DIR = pathlib.Path(__file__).parent.parent / "results_all"

alg_list = [("bracha","BrachaPeer"), ("imbsraynal", "ImbsRaynalPeer"), ("alg24", "Alg24Peer"), ("alg23", "Alg23Peer"), ("cool", "COOLPeer")]
n_list = [100]
f_list = [19, 20, 25, 33, 40]
p_list = [100, 90, 80, 70, 60, 50]
tests = 1000 

comb = {
    'alg23': {'msg_type': ['ack'], 'combinations': ['silent', 'opposite']}, 
    'alg24': {'msg_type': ['ack', 'vote1', 'vote2'], 'combinations': [['silent', 'same', 'same'], ['opposite', 'same', 'same'], ['same', 'silent', 'same'], ['silent', 'silent', 'same'], ['opposite', 'silent', 'same'], ['same', 'opposite', 'same'], ['silent', 'opposite', 'same'], ['opposite', 'opposite', 'same'], ['same', 'same', 'silent'], ['silent', 'same', 'silent'], ['opposite', 'same', 'silent'], ['same', 'silent', 'silent'], ['silent', 'silent', 'silent'], ['opposite', 'silent', 'silent'], ['same', 'opposite', 'silent'], ['silent', 'opposite', 'silent'], ['opposite', 'opposite', 'silent'], ['same', 'same', 'opposite'], ['silent', 'same', 'opposite'], ['opposite', 'same', 'opposite'], ['same', 'silent', 'opposite'], ['silent', 'silent', 'opposite'], ['opposite', 'silent', 'opposite'], ['same', 'opposite', 'opposite'], ['silent', 'opposite', 'opposite'], ['opposite', 'opposite', 'opposite']]}, 
    'bracha': {'msg_type': ['echo', 'ready'], 'combinations': [['same', 'silent'], ['silent', 'opposite'], ['silent', 'same'], ['opposite', 'opposite'], ['same', 'opposite'], ['silent', 'silent'], ['opposite', 'same'], ['opposite', 'silent']]}, 
    'imbsraynal': {'msg_type': ['witness'], 'combinations': ['silent', 'opposite']},
    'cool': {'msg_type': ['exchange',"ok1","ok2","done","yourpoint","mypoint"], 'combinations': [['silent','silent','silent','silent','silent','silent'], ['opposite','send','send','send','opposite','opposite']]}
}

def combination_to_string(l):
    if type(l) == str:
        return l
    x = ""
    for i in range(len(l)-1):
        x+=l[i]+"_"
    x+= l[-1]
    return x


def get_groups(n, p):
    group_size = n * p // 100
    all_node = [i for i in range(n)]
    random.shuffle(all_node)
    group_1 = all_node[:group_size]
    group_2 = all_node[group_size:]
    return group_1, group_2

def getByzantineVector(n,f):
    byzantine_nodes = random.sample(range(0, n), f)
    vec = [0 for _ in range(n)]
    for val in byzantine_nodes:
        vec[val] = 1
    return byzantine_nodes, vec

def getByzantineSender(vec):
    return random.choice(vec)

def getGlobalDelays_equal(n):
    x = []
    for i in range(1,21):
        for j in range(n*n//20):
            val = round(i*0.05,2)
            x.append(val)  
    random.shuffle(x)
        
    delays = {}
    counter = 0
    for i in range(n):
        delays[str(i)] = {}
        for j in range(n):
            delays[str(i)][str(j)] = x[counter]
            counter += 1
    return delays

def getGlobalDelays_distribution(n):
    delays = {}
    counter = 0
    for i in range(n):
        delays[str(i)] = {}
        for j in range(n):
            delays[str(i)][str(j)] = round(random.uniform(0.05, 2.0),2)
    return delays


for alg, alg_class in alg_list:
    file_name = f"{alg}.json"
    file_path = DIR_FILE_JSON / alg_class / file_name  
    n = n_list[0]

    for combination in comb[alg]['combinations']:
        experiments = {} 
        experiments["experiments"] = []
        file_name = f"{alg}_{combination_to_string(combination)}.json"
        file_path = DIR_FILE_JSON / alg_class / file_name
        for f in f_list:
            for p in p_list:
            
                exp = {}
                
                exp["parameters"] = {}
                exp["distribution"] = {}
                exp["topology"] = {}
                
                parameters = exp["parameters"]
                parameters["debug_prints"] = False
                parameters["n"] = n
                parameters["f"] = f
                parameters["percentage"] = p
                byz_vec, vec = getByzantineVector(n,f)
                parameters["byzantine_nodes"] = vec
                parameters["sender"] = getByzantineSender(byz_vec)
                group_0, group_1 = get_groups(n, p)
                parameters["group_0"] = group_0
                parameters["group_1"] = group_1
                parameters["combination"] = combination

                distribution = exp["distribution"]
                distribution["type"] = "GEOMETRIC"
                distribution["maxDelay"] = 10
                distribution["global_delay"] = 5
                distribution["global_delays_setting"] = "uniform"
                distribution["min_lambda"] = 0.05
                distribution["max_lambda"] = 0.2
                distribution["n"] = n
                distribution["links_delay"] = {}
                #distribution["links_delay"] = getGlobalDelays_distribution(n) 
                
                topology = exp["topology"]
                topology["type"] = "fullyComplete"
                topology["initialPeers"] = n
                topology["totalPeers"] = n 

                exp["logFile"] = f"results_all/{alg}/{combination_to_string(combination)}/n{n}_f{f}_p{p}.json"
                file_path_on_disk = RESULTS_DIR / alg / combination_to_string(combination) / f"n{n}_f{f}_p{p}.json"
                os.makedirs(os.path.dirname(file_path_on_disk), exist_ok=True)  
                with open(file_path_on_disk, 'w') as log_file:
                    json.dump({}, log_file)
                exp["tests"] = 50
                exp["rounds"] = 100
                exp["algorithm"] = alg
                exp["output_status"] = file_name
                exp["output_status_file"] = f"status_{alg}.txt"
                experiments["experiments"].append(exp)
    
        print(alg, alg_class)
        os.makedirs(os.path.dirname(file_path), exist_ok=True)  
        with open(file_path, 'w') as f:
            json.dump(experiments, f, indent=4)
    


