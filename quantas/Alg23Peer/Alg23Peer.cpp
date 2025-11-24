/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include "Alg23Peer.hpp"

string RED = "\033[31m";
string GREEN = "\033[32m";
string YELLOW = "\033[33m";
string BLUE = "\033[34m";
string MAGENTA = "\033[35m";
string CYAN = "\033[36m";
string RESET = "\033[0m";


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
	Alg23Peer::~Alg23Peer() {

	}

	Alg23Peer::Alg23Peer(const Alg23Peer& rhs) : Peer<Alg23Message>(rhs) {
		
	}

	Alg23Peer::Alg23Peer(long id) : Peer(id) {
		
	}

	void Alg23Peer::initParameters(const vector<Peer<Alg23Message>*>& _peers, json parameters) {
		const vector<Alg23Peer*> peers = reinterpret_cast<vector<Alg23Peer*> const&>(_peers);
		
		int f = parameters["f"];
		int n = parameters["n"];
		network_size = n;
		sender = parameters["sender"];
		percentage = parameters["percentage"];
		group_0 = parameters["group_0"].get<vector<interfaceId>>();
		group_1 = parameters["group_1"].get<vector<interfaceId>>();
		combination = parameters["combination"];

		is_byzantine = true;
		if (parameters["byzantine_nodes"][id()] == 0) is_byzantine = false;

		honest_nodes.clear();
		for (int i = 0; i<n; i++){
			if (parameters["byzantine_nodes"][i] == 0) honest_nodes.push_back(i);
		}

		debug_prints = false;
		if (parameters.contains("debug_prints")) debug_prints = parameters["debug_prints"];
		
		ack_ack_threshold = n-2*f;
		ack_delivery_threshold = n-f-1;
		//if (id() == 0) cout << "Node " << id() << " ack_ack_threshold: " << ack_ack_threshold << ", ack_delivery_threshold: " << ack_delivery_threshold << endl;

		delivered = false;
		is_first_propose = true;
		ack_msgs.clear();
		sent_ack_msgs.clear();
		
		finished_round = -1;
		final_value = -1;
		finishing_step = -1;
		total_msgs_sent = 0;
		
	}

	void Alg23Peer::performComputation() {

		for (const auto& m : sent_ack_msgs ){
			if (is_byzantine) printf("%s(ack, s:%ld, v:%d)%s  -  ", MAGENTA.c_str(), id(), 1-m, RESET.c_str());
			else printf("%s(ack, s:%ld, v:%d)%s  -  ", MAGENTA.c_str(), id(), m, RESET.c_str());
		}
		cout << endl;

		// ------------------------------ STEP 1: Propose -----------------------------------------
		if (is_byzantine && getRound() == 0 && id() == sender) {
			Alg23Message m0;
			m0.type = "propose";
			m0.source = id();
			m0.value = 0;

			Alg23Message m1;
			m1.type = "propose";
			m1.source = id();
			m1.value = 1;

			byzantine_broadcast(m0, m1, group_0, group_1);
			if (debug_prints) cout << RED << " sent byzantine propose messages" << RESET << endl;
		}
		// ----------------------------------------------------------------------------------------

		if (delivered) return;

		if (debug_prints) cout << "node_" << id() << " -------------------------------------" << endl;
		while (!inStreamEmpty()) {
			Packet<Alg23Message> newMsg = popInStream();
			Alg23Message m = newMsg.getMessage();
			addMsg(m);
			if (debug_prints) printf("%s<-- (%s, s:%ld, v:%d)%s\n", BLUE.c_str(), m.type.c_str(), m.source, m.value, RESET.c_str());

			
			// ------------------------------ STEP 2.1: Propose -> Ack ----------------------------
			if (m.type == "propose" && is_first_propose){
				Alg23Message ack_msg;

				// byzantine node
				if (is_byzantine && combination != "silent"){
					ack_msg.type = "ack";
					ack_msg.source = id();
					ack_msg.value = byzantine_value(combination, m.value);
					broadcast(ack_msg);

					//i send ack_msg.value but i add m.value otherwise i sends multiple acks
					sent_ack_msgs.push_back(m.value);
				}
				// honest node
				else if (!is_byzantine){
					ack_msg.type = "ack";
					ack_msg.source = id();
					ack_msg.value = m.value;
					broadcast(ack_msg);
					total_msgs_sent  += network_size;
					sent_ack_msgs.push_back(m.value);
				}
				is_first_propose = false;
				if (debug_prints) printf("%s--> (%s, s:%ld, v:%d)%s\n", GREEN.c_str(), ack_msg.type.c_str(), ack_msg.source, ack_msg.value, RESET.c_str());
			}
			// ------------------------------------------------------------------------------------


			if (m.type == "ack"){
				// ------------------------------ STEP 2.2: Ack -> Ack ----------------------------
				if ((count(ack_msgs, m.value) >= ack_ack_threshold) && (find(sent_ack_msgs.begin(), sent_ack_msgs.end(), m.value) == sent_ack_msgs.end())){
					Alg23Message ack_msg;

					// byzantine node
					if (is_byzantine && combination != "silent"){
						ack_msg.type = "ack";
						ack_msg.source = id();
						ack_msg.value = byzantine_value(combination, m.value);
						broadcast(ack_msg);

						//i send ack_msg.value but i add m.value otherwise i sends multiple acks
						sent_ack_msgs.push_back(m.value); 
					}
					// honest node
					else if (!is_byzantine){
						ack_msg.type = "ack";
						ack_msg.source = id();
						ack_msg.value = m.value;
						broadcast(ack_msg);
						total_msgs_sent  += network_size;
						sent_ack_msgs.push_back(ack_msg.value);
					}
					if (debug_prints) printf("%s--> (%s, s:%ld, v:%d)%s\n", GREEN.c_str(), ack_msg.type.c_str(), ack_msg.source, ack_msg.value, RESET.c_str());
				}
				// --------------------------------------------------------------------------------


				// ------------------------------ STEP 3: Commit ----------------------------------
				if ((count(ack_msgs, m.value) >= ack_delivery_threshold) && (delivered == false) && (!is_byzantine)){
					final_value = m.value;
					finished_round = getRound();
					delivered = true;
					if (sent_ack_msgs.size() == 1) finishing_step = 2;
					else finishing_step = 3;
					if (debug_prints) cout << RED << " DELIVERED value " << final_value << RESET << endl;
				}
				// --------------------------------------------------------------------------------
			}
	
		}

		if (debug_prints) {
			cout << " ack_msgs: [";
			for (const auto& p : ack_msgs) {
				cout << "(" << p.first << "," << p.second << ") ";
			}
			cout << "]" << endl;
			cout << "--------------------------------------------" << endl << endl;
		}
		
	}

	void Alg23Peer::endOfRound(const vector<Peer<Alg23Message>*>& _peers) {
		if (debug_prints) cout << "-------------------------------------------------- End of round " << getRound() << "--------------------------------------------------" << endl << endl;
	}

	Simulation<quantas::Alg23Message, quantas::Alg23Peer>* generateSim() {
        
        Simulation<quantas::Alg23Message, quantas::Alg23Peer>* sim = new Simulation<quantas::Alg23Message, quantas::Alg23Peer>;
        return sim;
    }
}