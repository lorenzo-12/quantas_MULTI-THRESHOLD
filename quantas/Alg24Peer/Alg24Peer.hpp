/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef Alg24Peer_hpp
#define Alg24Peer_hpp

#include <iostream>
#include "../Common/Peer.hpp"
#include "../Common/Simulation.hpp"

namespace quantas{

    using std::string; 
    using std::ostream;

    //
    // Example of a message body type
    //
    struct Alg24Message{
        
        string type;
        long source;
        int value;
        
    };

    //
    // Example Peer used for network testing
    //
    class Alg24Peer : public Peer<Alg24Message>{
    public:

        // ----------- Result parameters -----------
        bool is_byzantine;
        long sender;
        int percentage;
        vector<interfaceId> honest_nodes;
        vector<string> combination;

        int finished_round;
        int final_value;
        int finishing_step;
        bool debug_prints;
        int total_msgs_sent;
        int network_size;
        // -----------------------------------------

        // ----- Algorithm specific parameters -----
        int ack_delivery_threshold;
        int ack_vote1_threshold;
        int vote1_vote2_threshold;
        int vote2_vote2_threshold;
        int vote2_delivery_threshold;
        bool delivered;
        bool is_first_propose;

        bool ack_sent;
        bool vote1_sent;
        bool vote2_sent;
        int ack_sent_value = -1;
		int vote1_sent_value = -1;
		int vote2_sent_value = -1;
        map<long, int> ack_msgs;
        map<long, int> vote1_msgs;
        map<long, int> vote2_msgs;
        vector<interfaceId> group_0;
        vector<interfaceId> group_1;

        int count(const map<long, int>& s, int value){
            int counter = 0;
            for (const auto& p : s) {
                if (p.first == sender) continue;
                if (p.second == value) counter++;
            }
            return counter;
        }

        void addMsg(Alg24Message m) {
            if (m.type == "ack") {
                ack_msgs[m.source] = m.value;
            }
            if (m.type == "vote1") {
                vote1_msgs[m.source] = m.value;
            }
            if (m.type == "vote2") {
                vote2_msgs[m.source] = m.value;
            }
        }

        int addNodeResults(vector<int>& final_values_vec, vector<int>& final_times_vec, vector<int>& final_steps_vec, vector<int>& total_msgs_sent_vec) {
            final_values_vec.push_back(final_value);
            final_times_vec.push_back(finished_round);
            final_steps_vec.push_back(finishing_step);
            total_msgs_sent_vec.push_back(total_msgs_sent);
            return 0;
        }
        // -----------------------------------------


        // methods that must be defined when deriving from Peer
        Alg24Peer                             (long);
        Alg24Peer                             (const Alg24Peer &rhs);
        ~Alg24Peer                            ();

        // initialize the configuration of the system
        void                 initParameters(const vector<Peer<Alg24Message>*>& _peers, json parameters);
        // perform one step of the Algorithm with the messages in inStream
        void                 performComputation ();
        // perform any calculations needed at the end of a round such as determine throughput (only ran once, not for every peer)
        void                 endOfRound         (const vector<Peer<Alg24Message>*>& _peers);
    };

    Simulation<quantas::Alg24Message, quantas::Alg24Peer>* generateSim();
}
#endif /* Alg24Peer_hpp */
