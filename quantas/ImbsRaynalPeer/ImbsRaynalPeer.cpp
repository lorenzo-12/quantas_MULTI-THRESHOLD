/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#include "ImbsRaynalPeer.hpp"

int byzantine_value(string behavior, int v) {
	if (behavior == "opposite") {
		return 1 - v;
	}
	// default behavior: same value
	return v;
}

namespace quantas {

	//
	// Imbs-Raynal Channel definitions
	//
	ImbsRaynalPeer::~ImbsRaynalPeer() {

	}

	ImbsRaynalPeer::ImbsRaynalPeer(const ImbsRaynalPeer& rhs) : Peer<ImbsRaynalMessage>(rhs) {
		
	}

	ImbsRaynalPeer::ImbsRaynalPeer(long id) : Peer(id) {
		
	}

	void ImbsRaynalPeer::initParameters(const vector<Peer<ImbsRaynalMessage>*>& _peers, json parameters) {
		const vector<ImbsRaynalPeer*> peers = reinterpret_cast<vector<ImbsRaynalPeer*> const&>(_peers);
		
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
		
		witness_threshold = n -2*f;
		delivery_threshold = n-f;
		delivered = false;
		received_init.clear();
		received_witness.clear();
		broadcast_witness.clear();
		
		finished_round = -1;
		final_value = -1;
		finishing_step = -1;
		total_msgs_sent = 0;
		
	}

	void ImbsRaynalPeer::performComputation() {

		// ------------------------------ STEP 0: Init --------------------------------------------
		if (is_byzantine && getRound() == 0 && id() == sender) {
			ImbsRaynalMessage m0;
			m0.type = "init";
			m0.source = id();
			m0.value = 0;

			ImbsRaynalMessage m1;
			m1.type = "init";
			m1.source = id();
			m1.value = 1;

			byzantine_broadcast(m0, m1, group_0, group_1);
			//cout << " sent byzantine init messages" << endl;
		}
		// ----------------------------------------------------------------------------------------

		if (debug_prints) cout << "node_" << id() << " -------------------------------------" << endl;
		while (!inStreamEmpty()) {
			Packet<ImbsRaynalMessage> newMsg = popInStream();
			ImbsRaynalMessage m = newMsg.getMessage();
			if (debug_prints) printf("<-- (%s, %ld, %d) from %ld\n", m.type.c_str(), m.source, m.value, newMsg.sourceId());
		
			// ------------------------------ STEP 1.1: Witness -----------------------------------
			if (m.type == "init"){
				if (!contains(received_init, m.source) && !contains(broadcast_witness, m.source)) {
					ImbsRaynalMessage msg;

					// byzantine node
					if (is_byzantine && combination != "silent"){
						msg.type = "witness";
						msg.source = m.source;
						msg.value = byzantine_value(combination, m.value);
						broadcast(msg);
					}					
					// honest node
					else if (!is_byzantine) {
						msg.type = "witness";
						msg.source = m.source;
						msg.value = m.value;
						broadcast(msg);
						total_msgs_sent  += network_size;
					}

					broadcast_witness.push_back(make_pair(m.source, m.value));
					if (debug_prints) printf("--> (%s, %ld, %d)\n", msg.type.c_str(), msg.source, msg.value);
				}
				received_init.push_back(make_pair(m.source, m.value));
			}
			// ------------------------------------------------------------------------------------

			if (m.type == "witness"){
				// ------------------------------ STEP 1.2: Witness -------------------------------
				received_witness.push_back(make_pair(m.source, m.value));
				if( (check_witness(m.source, m.value)!=-1) && (!contains(broadcast_witness, m.source, m.value))) {

					// byzantine node
					if (is_byzantine && combination != "silent") {
						ImbsRaynalMessage msg;
						msg.type = m.type;
						msg.source = m.source;
						msg.value = byzantine_value(combination, m.value);
						broadcast(msg);
					}
					// honest node
					else if (!is_byzantine) {
						broadcast(m);
						total_msgs_sent  += network_size;
					}
					
					broadcast_witness.push_back(make_pair(m.source, m.value));
					if (debug_prints) printf("--> (%s, %ld, %d)\n", m.type.c_str(), m.source, m.value);
				}
				// --------------------------------------------------------------------------------


				// ------------------------------ STEP 2: Delivery --------------------------------
				if ((check_delivery(m.source, m.value)!=-1) && (delivered==false) && !is_byzantine) {
					delivered = true;
					finished_round = getRound();
					final_value = m.value;
					if (broadcast_witness.size() == 1) finishing_step = 2;
					else finishing_step = 3;
					if (debug_prints) cout << " DELIVERED value " << final_value << " in round " << finished_round << endl;
				}
				// --------------------------------------------------------------------------------
			}
		}

		if (debug_prints) {
			cout << "  received_init: [";
			for (const auto& p : received_init) {
				cout << "(" << p.first << "," << p.second << "), ";
			}
			cout << "]" << endl;
			cout << "  received_witness: [";
			for (const auto& p : received_witness) {
				cout << "(" << p.first << "," << p.second << "), ";
			}
			cout << "]" << endl;
			cout << "  broadcast_witness: [";
			for (const auto& p : broadcast_witness) {
				cout << "(" << p.first << "," << p.second << "), ";
			}
			cout << "]" << endl;
			cout << "--------------------------------------------" << endl << endl;
		}
			
		

	}

	void ImbsRaynalPeer::endOfRound(const vector<Peer<ImbsRaynalMessage>*>& _peers) {
		if (debug_prints) cout << "-------------------------------------------------- End of round " << getRound() << "--------------------------------------------------" << endl << endl;
	}

	Simulation<quantas::ImbsRaynalMessage, quantas::ImbsRaynalPeer>* generateSim() {
        
        Simulation<quantas::ImbsRaynalMessage, quantas::ImbsRaynalPeer>* sim = new Simulation<quantas::ImbsRaynalMessage, quantas::ImbsRaynalPeer>;
        return sim;
    }
}