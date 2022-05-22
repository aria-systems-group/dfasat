#include "state_merger.h"
#include "evaluate.h"
#include "evaluation_factory.h"
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <map>
#include <stdio.h>
#include <gsl/gsl_cdf.h>

#include "parameters.h"
#include "state_driven.h"
#include "num_count.h"


REGISTER_DEF_TYPE(state_driven);
REGISTER_DEF_DATATYPE(state_data);

state_data::state_data(){
    num_accepting = 0;
    num_rejecting = 0;
    accepting_paths = 0;
    rejecting_paths = 0;
};

void state_data::print_transition_label(iostream& output, int symbol){
    output << "count data";
};

void state_data::print_state_label(iostream& output){
    output << num_accepting;
};

void state_data::read_from(int type, int index, int length, int symbol, string data){
    if(type == 1){
        accepting_paths++;
    } else {
        rejecting_paths++;
    }
};

void state_data::read_to(int type, int index, int length, int symbol, string data){
    if(type == 1){
        if(length == index+1){
            num_accepting++;
        }
    } else {
        if(length == index+1){
            num_rejecting++;
        }
    }
};

void state_data::update(evaluation_data* right){
    state_data* other = reinterpret_cast<state_data*>(right);
    num_accepting += other->num_accepting;
    num_rejecting += other->num_rejecting;
    accepting_paths += other->accepting_paths;
    rejecting_paths += other->rejecting_paths;
};

void state_data::undo(evaluation_data* right){
    state_data* other = reinterpret_cast<state_data*>(right);
    num_accepting -= other->num_accepting;
    num_rejecting -= other->num_rejecting;
    accepting_paths -= other->accepting_paths;
    rejecting_paths -= other->rejecting_paths;
};

bool state_driven::consistency_check(evaluation_data* left, evaluation_data* right){
    state_data* l = reinterpret_cast<state_data*>(left);
    state_data* r = reinterpret_cast<state_data*>(right);

    if(l->num_accepting != 0 && r->num_rejecting != 0){ return false; }
    if(l->num_rejecting != 0 && r->num_accepting != 0){ return false; }

    return true;
};

/* default evaluation, count number of performed merges */
bool state_driven::consistent(state_merger *merger, apta_node* left, apta_node* right){
    if(inconsistency_found) return false;

    if(evaluation_function::consistent(merger, left, right) == false) return false;

    state_data* l = (state_data*)left->data;
    state_data* r = (state_data*)right->data;

    if(l->num_accepting != 0 && r->num_rejecting != 0){ inconsistency_found = true; return false; }
    if(l->num_rejecting != 0 && r->num_accepting != 0){ inconsistency_found = true; return false; }

    // TODO: ADD compatibility check for state transitions / guards
    // For now only merge that have same label
    if (left->label != right->label) {inconsistency_found = true; return false; }
    typedef one_vs_one_trainer<any_trainer<sample_type> > ovo_trainer;

    return true;
};

void state_driven::update_score(state_merger *merger, apta_node* left, apta_node* right){
  num_merges += 1;
};

int state_driven::compute_score(state_merger *merger, apta_node* left, apta_node* right){
  return num_merges;
};

void state_driven::reset(state_merger *merger){
  num_merges = 0;
  evaluation_function::reset(merger);
  compute_before_merge=false;
};


int state_data::sink_type(apta_node* node){
    if(!USE_SINKS) return -1;

    if (is_rejecting_sink(node)) return 0;
    if (is_accepting_sink(node)) return 1;
    return -1;
};

bool state_data::sink_consistent(apta_node* node, int type){
    if(!USE_SINKS) return true;

    if(type == 0) return is_rejecting_sink(node);
    if(type == 1) return is_accepting_sink(node);

    return true;
};

int state_data::num_sink_types(){
    if(!USE_SINKS) return 0;

    // accepting or rejecting
    return 2;
};

