#ifndef _SAFETY_H_
#define _SAFETY_H_

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <set>
#include <queue>

#include <yaml-cpp/yaml.h>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>

// Must define the class first
// Otherwise typedef would fail
class SafetyDFANode;
class SafetyDFA;
#include "apta.h"

typedef std::map<std::string, SafetyDFANode*> FormulaNodeMap;
typedef std::map<SafetyDFANode*, std::string> NodeFormulaMap;
typedef std::map<std::string, boost::function<bool(bool, bool)>> OperatorMap;



struct APTAEdge {
    apta_node* prev;
    std::string action;
    apta_node* curr;
};



struct SafetyDFAEdge {
    SafetyDFANode* prev;
    std::string action;
    SafetyDFANode* curr;
};



/**
 * @brief DFA Node for Safety LTL
 **/
class SafetyDFANode {
public:
    // Getter and Setters of States
    /** Name **/
    void setName(std::string name) { this->name = name; }
    std::string getName() { return name; }

    /** Transition Distribution **/
    void setTransDistribution(bool transDistribution) {
        this->transDistribution = transDistribution;
    }
    bool getTransDistribution() { return transDistribution; }

    /** Accepting State **/
    void setAcceptState(bool acceptState) { this->acceptState = acceptState; }
    bool isAccept() { return acceptState; }

    /** Incoming Edges **/
    SafetyDFANode* getIncomingNode(std::string formula) {
        // not found
        if ( ins.find(formula) == ins.end() ) {
            return NULL;
        }

        // found
        return ins[formula];
    }

    void setIncomingEdge(std::string formula, SafetyDFANode* src_node) {
        ins.insert(std::pair<std::string, SafetyDFANode*> (formula, src_node));
        insInv.insert(std::pair<SafetyDFANode*, std::string>(src_node, formula));
    }

    /** Outcoming Edges **/
    SafetyDFANode* getOutgoingNode(std::string formula) {
        // not found
        if ( outs.find(formula) == outs.end() ) {
            return NULL;
        }

        // found
        return outs[formula];
    }

    void setOutgoingEdge(std::string formula, SafetyDFANode* next_node) {
        outs.insert(std::pair<std::string, SafetyDFANode*> (formula, next_node));
        outsInv.insert(std::pair<SafetyDFANode*, std::string>(next_node, formula));
    }

    std::vector<std::string> getOutgoingFormulas() {
        std::vector<std::string> formulas;
        for (FormulaNodeMap::iterator it=outs.begin(); it!=outs.end(); ++it)
            formulas.push_back(it->first);
        return formulas;
    }

    /** Get List of Destination Node Pointers **/
    std::vector<SafetyDFANode*> getDestNodes() {
        std::vector<SafetyDFANode*> l;
        for (FormulaNodeMap::const_iterator it=outs.begin(); it!=outs.end(); ++it) {
            l.push_back(it->second);
        }
        return l;
    }

    /** Get List of Destination Node Names **/
    std::vector<std::string> getDestNodeNames() {
        std::vector<std::string> names;
        for (FormulaNodeMap::const_iterator it=outs.begin(); it!=outs.end(); ++it) {
            names.push_back(it->first);
        }
        return names;
    }

    /** Get List of Source Node Pointers **/
    std::vector<SafetyDFANode*> getSrcNodes() {
        std::vector<SafetyDFANode*> l;
        for (FormulaNodeMap::const_iterator it=ins.begin(); it!=ins.end(); ++it) {
            l.push_back(it->second);
        }
        return l;
    }

    /** Get List of Source Node Names **/
    std::vector<std::string> getSrcNodeNames() {
        std::vector<std::string> names;
        for (FormulaNodeMap::const_iterator it=ins.begin(); it!=ins.end(); ++it) {
            names.push_back(it->first);
        }
        return names;
    }

    /** Get Formula to get to the Destination Node **/
    std::string getFormulaFromDestNode(SafetyDFANode* destNode) {
        return outsInv[destNode];
    }

    /** Get Formula used to get to the current node from the Source Node **/
    std::string getFormulaFromSrcNode(SafetyDFANode* srcNode) {
        return insInv[srcNode];
    }

    // Additional States
    bool hasEdge() { if (outs.size() > 0) return true; return false; }
    bool isSink() { return hasEdge(); }
    int numOutEdges() { return outs.size(); }
    int numInEdges() { return ins.size(); }

private:
    /** Incoming Edges Key: Edge Symbol, Value: Node* **/
    FormulaNodeMap ins;
    NodeFormulaMap insInv;

    /** Outgoing Edges Key: Edge Symbol, Value: Node* **/
    FormulaNodeMap outs;
    NodeFormulaMap outsInv;

    /** unique number assigned to each node **/
    std::string name;

    /** Transition Distribution **/
    // TODO: I'm not sure if the type should be bool or string
    bool transDistribution;

    /** Accepting State **/
    bool acceptState;
};



class SafetyDFA {
public:
    /**
     * @brief Constructor
     **/
    SafetyDFA(){
        root = NULL;
        initializedAPTA = false;
    }

    SafetyDFA(std::string filename,
              int numAlgorithm = 0,
              std::vector<std::string> booleanOperators = {}) {
        root = NULL;
        initializedAPTA = false;
        readYaml(filename, booleanOperators);
    }

    /**
     * @brief read in yaml file
     **/
    void readYaml(std::string filename,
                  std::vector<std::string> booleanOperators = {});

    /**
     * @brief print nodes and edges to terminal
     **/
    void printTree();

    /**
     * @brief choose which safety checking algorithm to use
     **/
    void chooseAlgorithm(int numAlgorithm) { this->numAlgorithm = numAlgorithm; };

    /**
     * @brief check safety when merging any left and right node
     **/
    bool preCheckSafety(apta* aut, apta_node* left, apta_node* right);

    /**
     * @brief check safety after merging two nodes and its subtrees
     **/
    bool postCheckSafety(apta* aut, apta_node* left, apta_node* right);

    /**
     * @brief Initialize the APTA with safety states
     **/
    void initializeAPTA(apta* aut);

    /** Getters **/

    /** Returns extracted atomic propositions **/
    std::set<std::string> getAlphabet() { return alphabet; }

    bool isSafeSymbols(vector<string> symbols);

    void setAlphabet(vector<string> alphabet);


private:
    /**
     * @brief Check whether a APTA satisfies safety
     **/
    bool _runPolynomialAlgorithm(apta* aut, apta_node* left, apta_node* right);

    /**
     * @brief check whether a APTA satisfies safety
     **/
    bool _runGreedyAlgorithm(apta* aut, apta_node* left, apta_node* right);

    /**
     * @brief Check if given nodeName is contained in the nodeMap
     **/
    void _throwErrorIfNodeNotInDFA(std::string nodeName);

    /**
     * @brief Chceck if a set of node (ql, qs) is included in visited
     **/
    bool _checkIfVisitedPair(apta_node*  ql, SafetyDFANode* qs,
                             std::set<std::pair<apta_node*, SafetyDFANode*>> visited);

    /**
     * @brief Check if there is no more transition from current node
     **/
    bool _noValidTransitions(std::vector<bool> valids);

    /**
     * @brief Checks whether the transition to the next state is valid or not
     **/
    bool _nextAPTANodeConsistent(
        std::vector<std::pair<apta_node*, SafetyDFANode*>> transitions);

    /** Extract Symbols (a subset of alphabet) from APTA **/
    std::set<std::string> _extractSymbols(apta*  aut);

    /** Member variables **/


    /** DFA's root **/
    SafetyDFANode* root;

    /** Map Node Name to the Node Pointer **/
    std::map<std::string, SafetyDFANode*> nodeMap;

    /**
     * Alphabet extracted from yaml file.
     * Generally, Alphabet is a power set of
     * atomic propositions (boolean predicates).
     * In this implementation, we only deal with the alphabet
     * that is consisted of single atomic propositions.
     **/
    std::set<std::string> alphabet;

    /** Alphabet Size loaded from yaml file **/
    int alphabetSize;

    /** No. of State loaded from yaml file **/
    int numStates;

    /** Final Transition Symbol loaded from yaml file **/
    std::string finalTransitionSym;

    /** Empty Transition Symbol loaded from yaml file **/
    std::string emptyTransitionSym;

    /** Start State loaded from yaml file **/
    std::string startState;

    /** Smooth Transitions loaded from yaml file **/
    bool smoothTransitions;

    /** No. of chosen algorithm **/
    int numAlgorithm;

    /** Whether the APTA is initialized with safety nodes **/
    bool initializedAPTA;
};



#endif /* _SAFETY_H_ */
