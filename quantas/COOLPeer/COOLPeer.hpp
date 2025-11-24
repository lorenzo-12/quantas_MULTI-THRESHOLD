/*
Copyright 2022

This file is part of QUANTAS.
QUANTAS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
QUANTAS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with QUANTAS. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef COOLPeer_hpp
#define COOLPeer_hpp

#include <iostream>
#include "../Common/Peer.hpp"
#include "../Common/Simulation.hpp"

namespace quantas{

    using std::string; 
    using std::ostream;

    //
    // Example of a message body type
    //
    struct COOLMessage{
        
        string type;
        long source;
        int value;
        
    };

    //
    // Example Peer used for network testing
    //
    class COOLPeer : public Peer<COOLMessage>{
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

        int fx;
        int ok1_threshold;
        int ok2_threshold;
        int ok2_done_threshold;
        int done_done_threshold;
        int dispersal_termination_threshold;
        int mypoint_threshold;
        int decode_threshold;

        bool sent_ok1 = false;
        bool sent_ok2 = false;
        bool sent_done = false;
        bool sent_mypoint = false;
        bool dispersal_termination = false;
        bool decoded = false;
        bool delivered = false;

        vector<interfaceId> A1;
        vector<interfaceId> A2;
        vector<interfaceId> ok2_msgs;
        vector<interfaceId> done_msgs;
        map<long, int> M;
        map<long, int> S;
        vector<interfaceId> group_0;
        vector<interfaceId> group_1;

        bool check_ok1(){
            if (A1.size() >= ok1_threshold) return true;
            return false;
        }

        bool check_ok2(){
            if (A2.size() >= ok2_threshold) return true;
            return false;
        }

        bool check_done(){
            if (sent_ok2 && A2.size() >= ok2_done_threshold) return true;
            if (done_msgs.size() >= done_done_threshold) return true;
            return false;
        }

        bool check_dispersal_termination(){
            if (done_msgs.size() >= dispersal_termination_threshold) return true;
            return false;
        }

        int check_mypoint(){
            unordered_map<int, int> freq;
            for (const auto& m : M) {
                int v = m.second;
                int c = ++freq[v];
                if (c >= mypoint_threshold) return v;
            }
            return -1;
        }

        int check_decode(){
            unordered_map<int, int> freq;
            for (const auto& m : S) {
                int v = m.second;
                int c = ++freq[v];
                if (c >= decode_threshold) return v;
            }
            return -1;
        }

        int addNodeResults(vector<int>& final_values_vec, vector<int>& final_times_vec, vector<int>& final_steps_vec, vector<int>& total_msgs_sent_vec) {
            final_values_vec.push_back(final_value);
            final_times_vec.push_back(finished_round);
            final_steps_vec.push_back(7); // COOL always finishes in step 7
            total_msgs_sent_vec.push_back(total_msgs_sent);
            return 0;
        }


        // methods that must be defined when deriving from Peer
        COOLPeer                             (long);
        COOLPeer                             (const COOLPeer &rhs);
        ~COOLPeer                            ();

        // initialize the configuration of the system
        void                 initParameters(const vector<Peer<COOLMessage>*>& _peers, json parameters);
        // perform one step of the Algorithm with the messages in inStream
        void                 performComputation ();
        // perform any calculations needed at the end of a round such as determine throughput (only ran once, not for every peer)
        void                 endOfRound         (const vector<Peer<COOLMessage>*>& _peers);
    };

    Simulation<quantas::COOLMessage, quantas::COOLPeer>* generateSim();
}
#endif /* COOLPeer_hpp */
