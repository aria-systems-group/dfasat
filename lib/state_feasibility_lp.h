#ifndef _STATE_FEASIBILITY_LP_H_
#define _STATE_FEASIBILITY_LP_H_

#include <iostream>
#include <map>
#include <vector>
#include <glpk.h>

// #include <boost/function.hpp>
// #include <boost/lexical_cast.hpp>

// Must define the class first
// Otherwise typedef would fail
class StateFeasibilityLP;
#include "apta.h"

class StateFeasibilityLP {
public:
    /**
     * @brief Constructor
     **/
    StateFeasibilityLP(){}

    /**
     * @brief Check if the merge between left and right nodes is consistent by
     * checking if a set of states in child nodes are linearly separable
     */
    bool isFeasible(apta* aut, apta_node* left, apta_node* right);
    bool areSetsSeparable(list<vector<vector<double>>> Xs);

private:
    /** Test **/
    bool hello_world;
};



#endif /* _STATE_FEASIBILITY_LP_H_ */
