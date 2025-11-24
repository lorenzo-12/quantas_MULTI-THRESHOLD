/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include "COOLPeer.hpp"
#include <cmath>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

int byzantine_value(string behavior, int v) {
	if (behavior == "opposite") {
		return 1 - v;
	}
	// default behavior: same value
	return v;
}

namespace quantas {

	//
	// Example Channel definitions
	//
	COOLPeer::~COOLPeer() {

	}

	COOLPeer::COOLPeer(const COOLPeer& rhs) : Peer<COOLMessage>(rhs) {
		
	}

	COOLPeer::COOLPeer(long id) : Peer(id) {
		
	}

	void COOLPeer::initParameters(const vector<Peer<COOLMessage>*>& _peers, json parameters) {
		const vector<COOLPeer*> peers = reinterpret_cast<vector<COOLPeer*> const&>(_peers);
		
		int f = parameters["f"];
		int n = parameters["n"];
		network_size = n;
		sender = parameters["sender"];
		percentage = parameters["percentage"];
		group_0 = parameters["group_0"].get<vector<interfaceId>>();
		group_1 = parameters["group_1"].get<vector<interfaceId>>();
		combination = parameters["combination"].get<vector<string>>();

		is_byzantine = true;
		if (parameters["byzantine_nodes"][id()] == 0) is_byzantine = false;

		honest_nodes.clear();
		for (int i = 0; i<n; i++){
			if (parameters["byzantine_nodes"][i] == 0) honest_nodes.push_back(i);
		}

		debug_prints = false;
		if (parameters.contains("debug_prints")) debug_prints = parameters["debug_prints"];

		ok1_threshold = n - f;
		ok2_threshold = n - f;
		ok2_done_threshold = 2*f + 1;
		done_done_threshold = f+1;
		dispersal_termination_threshold = 2*f+1;
		mypoint_threshold = f+1;
		int poly_degree = static_cast<int>(std::floor(f/3.0));
		decode_threshold = f+1+poly_degree;

		sent_ok1 = false;
		sent_ok2 = false;
		sent_done = false;
		sent_mypoint = false;
		dispersal_termination = false;
		decoded = false;
		delivered = false;

		A1.clear();
		A2.clear();
		ok2_msgs.clear();
		done_msgs.clear();
		M.clear();
		S.clear();

		finished_round = -1;
		final_value = -1;
		total_msgs_sent = 0;

		if (find(group_0.begin(), group_0.end(), id()) != group_0.end()) fx = 0;
		else fx = 1;

	}

	void COOLPeer::performComputation() {

		// ------------------------------ BRB --------------------------------------------------
		if (is_byzantine && getRound() == 0 && id() == sender) {
			COOLMessage m0;
			m0.source = id();
			m0.type = "f(x)";
			m0.value = 0;

			COOLMessage m1;
			m1.source = id();
			m1.type = "f(x)";
			m1.value = 1;

			byzantine_broadcast(m0, m1, group_0, group_1);
			if (debug_prints) cout << " sent byzantine send messages" << endl;
		}
		// ----------------------------------------------------------------------------------------

		if (debug_prints) cout << "node_" << id() << " -------------------------------------" << endl;
		while (!inStreamEmpty()) {
			Packet<COOLMessage> newMsg = popInStream();
			COOLMessage m = newMsg.getMessage();
			if (debug_prints) printf("<-- (%s, %ld, %d)\n", m.type.c_str(), m.source, m.value);

			if (m.type == "f(x)") {
				fx = m.value;
				COOLMessage exchange_m;

				// byzantine node 
				if (is_byzantine && combination[0] != "silent"){
					exchange_m.source = id();
					exchange_m.type = "exchange";
					exchange_m.value = byzantine_value(combination[0], fx);
					broadcast(exchange_m);
				}
				// honest node
				else if (!is_byzantine){
					exchange_m.source = id();
					exchange_m.type = "exchange";
					exchange_m.value = fx;
					broadcast(exchange_m);
					total_msgs_sent  += network_size;
				}
				if (debug_prints) printf("--> (%s, %ld, %d)\n", exchange_m.type.c_str(), exchange_m.source, exchange_m.value);
			}
			else if (m.type == "exchange" && m.value == fx) A1.push_back(m.source);
			else if (m.type == "ok1" && std::find(A1.begin(), A1.end(), m.source) != A1.end()) A2.push_back(m.source);
			else if (m.type == "ok2") ok2_msgs.push_back(m.source);
			else if (m.type == "done") done_msgs.push_back(m.source);
			else if (m.type == "yourpoint") M[m.source] = m.value;
			else if (m.type == "mypoint") S[m.source] = m.value;

			// ------------------------------ DISPERSAL --------------------------------------------

			if (sent_ok1 == false && check_ok1()){
				COOLMessage ok1_m;

				// byzantine node
				if (is_byzantine && combination[1] != "silent"){
					ok1_m.source = id();
					ok1_m.type = "ok1";
					broadcast(ok1_m);
				}
				// honest node
				else if (!is_byzantine){
					ok1_m.source = id();
					ok1_m.type = "ok1";
					broadcast(ok1_m);
					total_msgs_sent  += network_size;
				}

				sent_ok1 = true;
				if (debug_prints) printf("--> (%s, %ld, %d)\n", ok1_m.type.c_str(), ok1_m.source, ok1_m.value);
			}

			if (sent_ok2 == false && check_ok2()){
				COOLMessage ok2_m;

				// byzantine node
				if (is_byzantine && combination[2] != "silent"){
					ok2_m.source = id();
					ok2_m.type = "ok2";
					broadcast(ok2_m);
					total_msgs_sent  += network_size;
				}
				// honest node
				else if (!is_byzantine){
					ok2_m.source = id();
					ok2_m.type = "ok2";
					broadcast(ok2_m);
					total_msgs_sent  += network_size;
				}

				sent_ok2 = true;
				if (debug_prints) printf("--> (%s, %ld, %d)\n", ok2_m.type.c_str(), ok2_m.source, ok2_m.value);
			}

			if (sent_done == false && check_done()){
				COOLMessage done_m;

				// byzantine node
				if (is_byzantine && combination[3] != "silent"){
					done_m.source = id();
					done_m.type = "done";
					broadcast(done_m);
				}
				// honest node
				else if (!is_byzantine){
					done_m.source = id();
					done_m.type = "done";
					broadcast(done_m);
					total_msgs_sent  += network_size;
				}

				sent_done = true;
				if (debug_prints) printf("--> (%s, %ld, %d)\n", done_m.type.c_str(), done_m.source, done_m.value);
			}

			if (dispersal_termination == false && check_dispersal_termination()){
				dispersal_termination = true;
				if (sent_ok2 == true){ 
					// we are in the case with f(x)
					COOLMessage yourpoint_m;

					// byzantine node
					if (is_byzantine && combination[4] != "silent"){
						yourpoint_m.source = id();
						yourpoint_m.type = "yourpoint";
						yourpoint_m.value = byzantine_value(combination[4], fx);
						broadcast(yourpoint_m);
					}
					// honest node
					else if (!is_byzantine){
						yourpoint_m.source = id();
						yourpoint_m.type = "yourpoint";
						yourpoint_m.value = fx;
						broadcast(yourpoint_m);
						total_msgs_sent  += network_size;
					}
					
					if (debug_prints) printf("--> (%s, %ld, %d)\n", yourpoint_m.type.c_str(), yourpoint_m.source, yourpoint_m.value);
				}
				else {
					// do nothing (we are in the case with ⊥)
				}
			}
			
			// ------------------------------ DATA DISSEMINATION -----------------------------------

			if (sent_mypoint == false && check_mypoint()!=-1){
				COOLMessage mypoint_m;

				// byzantine node
				if (is_byzantine && combination[5] != "silent"){
					mypoint_m.source = id();
					mypoint_m.type = "mypoint";
					mypoint_m.value = byzantine_value(combination[5], check_mypoint());
					broadcast(mypoint_m);
				}
				// honest node
				else if (!is_byzantine){
					mypoint_m.source = id();
					mypoint_m.type = "mypoint";
					mypoint_m.value = check_mypoint();
					broadcast(mypoint_m);
					total_msgs_sent  += network_size;
				}

				sent_mypoint = true;
				if (debug_prints) printf("--> (%s, %ld, %d)\n", mypoint_m.type.c_str(), mypoint_m.source, mypoint_m.value);
			}

			if (decoded == false && check_decode()!=-1 && !is_byzantine){
				decoded = true;
				final_value = check_decode();
				finished_round = getRound();
				if (debug_prints) cout << "Node_" << id() << " DECODES value " << final_value << " in round " << finished_round << endl;
			}
		}
	}

	void COOLPeer::endOfRound(const vector<Peer<COOLMessage>*>& _peers) {
		if (debug_prints) cout << "-------------------------------------------------- End of round " << getRound() << "--------------------------------------------------" << endl << endl;
	}

	Simulation<quantas::COOLMessage, quantas::COOLPeer>* generateSim() {
        
        Simulation<quantas::COOLMessage, quantas::COOLPeer>* sim = new Simulation<quantas::COOLMessage, quantas::COOLPeer>;
        return sim;
    }
}