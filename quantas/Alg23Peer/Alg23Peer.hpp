/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef Alg23Peer_hpp
#define Alg23Peer_hpp

#include <iostream>
#include "../Common/Peer.hpp"
#include "../Common/Simulation.hpp"

namespace quantas{

    using std::string; 
    using std::ostream;

    //
    // Example of a message body type
    //
    struct Alg23Message{
        
        string type;
        long source;
        int value;
        
    };

    //
    // Example Peer used for network testing
    //
    class Alg23Peer : public Peer<Alg23Message>{
    public:

        // ----------- Result parameters -----------
        bool is_byzantine;
        long sender;
        int percentage;
        vector<interfaceId> honest_nodes;
        string combination;

        int finished_round;
        int final_value;
        int finishing_step;
        bool debug_prints;
        int total_msgs_sent;
        int network_size;
        // -----------------------------------------

        // ----- Algorithm specific parameters -----
        int ack_ack_threshold;
        int ack_delivery_threshold;
        bool delivered;
        bool is_first_propose;

        vector<int> sent_ack_msgs;
        vector<pair<long, int>> ack_msgs;
        vector<interfaceId> group_0;
        vector<interfaceId> group_1;

        int count(const vector<pair<long, int>>& s, int value){
            int counter = 0;
            for (const auto& p : s) {
                if (p.first == sender) continue;
                if (p.second == value) counter++;
            }
            return counter;
        }

        void addMsg(Alg23Message m) {
            if (m.type == "ack") {
                ack_msgs.push_back(make_pair(m.source, m.value));
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
        Alg23Peer                             (long);
        Alg23Peer                             (const Alg23Peer &rhs);
        ~Alg23Peer                            ();

        // initialize the configuration of the system
        void                 initParameters(const vector<Peer<Alg23Message>*>& _peers, json parameters);
        // perform one step of the Algorithm with the messages in inStream
        void                 performComputation ();
        // perform any calculations needed at the end of a round such as determine throughput (only ran once, not for every peer)
        void                 endOfRound         (const vector<Peer<Alg23Message>*>& _peers);
    };

    Simulation<quantas::Alg23Message, quantas::Alg23Peer>* generateSim();
}
#endif /* Alg23Peer_hpp */
