/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef BrachaPeer_hpp
#define BrachaPeer_hpp

#include <iostream>
#include "../Common/Peer.hpp"
#include "../Common/Simulation.hpp"

namespace quantas{

    using std::string; 
    using std::ostream;

    //
    // Example of a message body type
    //
    struct BrachaMessage{
        
        string type;
        long source;
        int value;
    };

    //
    // Example Peer used for network testing
    //
    class BrachaPeer : public Peer<BrachaMessage>{
    public:

        // ----------- Result parameters -----------
        bool is_byzantine;
        long sender;
        int percentage;
        vector<interfaceId> honest_nodes;
        vector<string> combination;

        int finished_round;
        int final_value;
        bool debug_prints;
        int total_msgs_sent;
        int network_size;
        // -----------------------------------------

        // ----- Algorithm specific parameters -----
        int echo_threshold;
        int ready_threshold;
        int delivery_threshold;
        bool sent_echo = false;
        bool sent_ready = false;
        bool delivered = false;
        int sent_echo_value = -1;
        int sent_ready_value = -1;
        map<long, int> echo_msgs;
        map<long, int> ready_msgs;
        vector<interfaceId> group_0;
        vector<interfaceId> group_1;

        int check_echo(){
            unordered_map<int, int> freq;
            for (const auto& m : echo_msgs) {
                int v = m.second;
                int c = ++freq[v];
                if (c >= echo_threshold) return v;
            }
            return -1;
        }

        int check_ready(){
            unordered_map<int, int> freq;
            for (const auto& m : ready_msgs) {
                int v = m.second;
                int c = ++freq[v];
                if (c >= ready_threshold) return v;
            }
            return -1;
        }
        int check_delivery(){
            unordered_map<int, int> freq;
            for (const auto& m : ready_msgs) {
                int v = m.second;
                int c = ++freq[v];
                if (c >= delivery_threshold) return v;
            }
            return -1;
        }

        inline int getReadyType(map<long, int> echo_msgs, int final_value, int echo_threshold){
            int count_echo = 0;
            for (const auto& m : echo_msgs) {
                if (m.second == final_value) count_echo++;
            }
            if (count_echo >= echo_threshold) return 1;
            return 2;
        }

        int addNodeResults(vector<int>& final_values_vec, vector<int>& final_times_vec, vector<int>& final_steps_vec, vector<int>& total_msgs_sent_vec) {
            final_values_vec.push_back(final_value);
            final_times_vec.push_back(finished_round);
            final_steps_vec.push_back(getReadyType(echo_msgs, final_value, echo_threshold));
            total_msgs_sent_vec.push_back(total_msgs_sent);
            return delivery_threshold;
        }
        // -----------------------------------------


        // methods that must be defined when deriving from Peer
        BrachaPeer                             (long);
        BrachaPeer                             (const BrachaPeer &rhs);
        ~BrachaPeer                            ();

        // initialize the configuration of the system
        void                 initParameters(const vector<Peer<BrachaMessage>*>& _peers, json parameters);
        // perform one step of the Algorithm with the messages in inStream
        void                 performComputation ();
        // perform any calculations needed at the end of a round such as determine throughput (only ran once, not for every peer)
        void                 endOfRound         (const vector<Peer<BrachaMessage>*>& _peers);
    };

    Simulation<quantas::BrachaMessage, quantas::BrachaPeer>* generateSim();
}
#endif /* BrachaPeer_hpp */
