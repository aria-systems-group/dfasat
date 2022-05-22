#include "safety.h"
#include "formal_language.h"


/** Public Member Functions **/

void SafetyDFA::readYaml(std::string filename,
                         std::vector<std::string> booleanOperators) {
    /**
     *
     **/

    if (filename == "") return;

    YAML::Node config;
    config = YAML::LoadFile(filename);

    if (booleanOperators.empty()) booleanOperators = {" & ", " | "};

    alphabetSize = config["alphabet_size"].as<int>();
    numStates = config["num_states"].as<int>();
    finalTransitionSym = config["final_transition_sym"].as<std::string>();
    emptyTransitionSym = config["empty_transition_sym"].as<std::string>();
    startState = config["start_state"].as<std::string>();
    smoothTransitions = config["smooth_transitions"].as<bool>();

    // Load Nodes
    YAML::Node nodes = config["nodes"];
    // Save Nodes as a NodeMap
    for(YAML::const_iterator it=nodes.begin();it!=nodes.end();++it) {

        std::string nodeName = it->first.as<std::string>();
        YAML::Node nodeData = it->second;

        SafetyDFANode* node = new SafetyDFANode();
        node->setName(nodeName);

        node->setAcceptState(nodeData["is_accepting"].as<bool>());

        // Add Nodes
        nodeMap.insert(std::pair<std::string, SafetyDFANode*>(nodeName, node));

        if (nodeName == startState) {
            root = new SafetyDFANode();
            root->setOutgoingEdge(emptyTransitionSym, node);
            node->setIncomingEdge(emptyTransitionSym, root);
        }
    }

    // Load Edges
    YAML::Node edgeNodes = config["edges"];
    // Add Edges to the NodeMap
    for(YAML::const_iterator itNode=edgeNodes.begin(); itNode!=edgeNodes.end(); ++itNode) {
        std::string srcNodeName = itNode->first.as<std::string>();
        YAML::Node edges = itNode->second;
        _throwErrorIfNodeNotInDFA(srcNodeName);

        for (YAML::const_iterator itEdge=edges.begin(); itEdge!=edges.end(); ++itEdge){
            std::string destNodeName = itEdge->first.as<std::string>();
            // Safety DFA's edges are consisted of "formulas" instead of "symbols"
            YAML::Node formulas = itEdge->second["symbols"];
            std::string formula = formulas[0].as<std::string>();

            std::vector<std::string> symbols;
            std::vector<std::string> booleans;
            separateFormula(formula, symbols, booleans, booleanOperators);
            for (std::string s : symbols) {
                std::string symbol = s;
                if (isStringFound(s.find("!"))) symbol = s.substr(1, s.size()-1);
                alphabet.insert(symbol);
            }

            _throwErrorIfNodeNotInDFA(destNodeName);

            // Add Edges
            nodeMap[srcNodeName]->setOutgoingEdge(formula, nodeMap[destNodeName]);
            nodeMap[destNodeName]->setIncomingEdge(formula, nodeMap[srcNodeName]);
        }
    }
}


void SafetyDFA::printTree(){
    if (root == NULL) {
        std::cout << "The tree is not yet constructed" << std::endl;
        return;
    }
    for (std::map<std::string, SafetyDFANode*>::const_iterator it=nodeMap.begin(); it!=nodeMap.end(); ++it) {
        std::string nodeName = it->first;
        SafetyDFANode* currNode = it->second;

        std::cout << nodeName << std::endl;
        std::vector<SafetyDFANode*> destNodes = currNode->getDestNodes();

        for (SafetyDFANode* destNode : destNodes) {
            std::string formula = currNode->getFormulaFromDestNode(destNode);
            std::cout << "      " << formula << " ->  " << destNode->getName() << std::endl;
        }
    }
}


bool SafetyDFA::preCheckSafety(apta* aut, apta_node* left, apta_node* right) {
    if (root == NULL) {
        // std::cerr << " root is NULL " << std::endl;
        return true;
    }

    if (numAlgorithm == 0) {
        return _runPolynomialAlgorithm(aut, left, right);
    }
    return true;
}


bool SafetyDFA::postCheckSafety(apta* aut, apta_node* left, apta_node* right) {
    // If Safety DFA is not given, just return true
    if (root == NULL) {
        // std::cerr << " root is NULL " << std::endl;
        return true;
    }

    if (numAlgorithm == 0) {
        return true;
    } else if (numAlgorithm == 1) {
        return _runGreedyAlgorithm(aut, left, right);
    } else {
        throw std::invalid_argument("Choose the right Safety Checking Algorithm");
    }
}


void SafetyDFA::setAlphabet(vector<string> alphabet) {
    this->alphabet.insert(alphabet.begin(), alphabet.end());
}


bool SafetyDFA::isSafeSymbols(vector<string> symbols) {
    if (root == NULL) {
        // std::cerr << " root is NULL " << std::endl;
        return true;
    }
    SafetyDFANode* q  = root->getDestNodes()[0];

    for (std::string symbol : symbols) {
        std::vector<std::string> formulas = q->getOutgoingFormulas();
        std::vector<bool> valids;

        for (std::string formula : formulas) {
            bool valid = satisfyFormula(formula, symbol, alphabet);
            valids.push_back(valid);

            if (!valid) continue;

            SafetyDFANode* next_qs = q->getOutgoingNode(formula);
        }

        int sumValid = 0;
        for (auto v : valids) sumValid += v;

        if (sumValid == 0) return false;
    }

    return true;
}


void SafetyDFA::initializeAPTA(apta* aut) {
    if (root == NULL) {
        // std::cerr << " root is NULL " << std::endl;
        return;
    }

    if (initializedAPTA) return;

    // Get All Symbols == Alphabet
    std::set<std::string> symbols = _extractSymbols(aut);
    alphabet.insert(symbols.begin(), symbols.end());

    std::queue<apta_node*> searchQueue;

    apta_node* aptaRoot = aut->root;
    SafetyDFANode* initNode = root->getDestNodes()[0];
    aptaRoot->setSafetyNode(initNode);

    searchQueue.push(aptaRoot);

    while (!searchQueue.empty()) {

        apta_node* ql = searchQueue.front(); searchQueue.pop();
        SafetyDFANode* qs = ql->getSafetyNode();

        // Get qs's edges
        std::vector<std::string> formulas = qs->getOutgoingFormulas();

        for (std::map<int, apta_guard*>::iterator it=ql->guards.begin(); it!=ql->guards.end(); ++it) {

            apta_node* next_ql_tmp = it->second->target;
            if (next_ql_tmp == 0) continue;

            int symbolId = it->first;
            std::string symbol =  aut->alphabet[symbolId];

            std::vector<bool> valids;
            apta_node* next_ql;

            for (std::string formula : formulas) {
                bool valid = satisfyFormula(formula, symbol, alphabet);
                valids.push_back(valid);

                if (!valid) continue;

                next_ql = next_ql_tmp;
                SafetyDFANode* next_qs = qs->getOutgoingNode(formula);
                next_ql->setSafetyNode(next_qs);
            }

            int sumValid = 0;
            for (auto v : valids) sumValid += v;

            // If there is one edge that transition to the next state,
            // push to the search queue
            if (sumValid == 1) {
                searchQueue.push(next_ql);
            } else {
                throw std::runtime_error("Unsafe Traces. Cannot initialize the Safety Check Algorithm");
            }
        }
    }

    initializedAPTA = true;
}


/** Private Member Functions **/


bool SafetyDFA::_runPolynomialAlgorithm(apta* aut, apta_node* left, apta_node* right) {
    std::string leftSafetyState = left->getSafetyNode()->getName();
    std::string rightSafetyState = right->getSafetyNode()->getName();

    return (leftSafetyState == rightSafetyState);
}


bool SafetyDFA::_runGreedyAlgorithm(apta* aut, apta_node* left, apta_node* right) {
    std::queue<APTAEdge> Q_live;
    std::queue<SafetyDFAEdge> Q_safe;
    std::set<std::pair<apta_node*, SafetyDFANode*>> visited;

    // There always is 1 transition edge from the root
    // to the initial node
    SafetyDFANode* initNode = root->getDestNodes()[0];

    APTAEdge aptaEdge = {aut->root, emptyTransitionSym, aut->root->find()};
    SafetyDFAEdge dfaEdge = {root, emptyTransitionSym, initNode};
    Q_live.push(aptaEdge);
    Q_safe.push(dfaEdge);
    visited.insert(std::make_pair(aut->root, root));

    while (!Q_live.empty()) {

        // pop current states
        APTAEdge aptaEdge = Q_live.front(); Q_live.pop();
        SafetyDFAEdge dfaEdge = Q_safe.front(); Q_safe.pop();
        apta_node* ql = aptaEdge.curr;
        SafetyDFANode* qs = dfaEdge.curr;

        // std::cout << "[" << aptaEdge.prev->find()->number << ", " << dfaEdge.prev->getName() << "]";
        // std::cout << "(" << aptaEdge.action << ")->";
        // std::cout << "[" << ql->find()->number << ", " << qs->getName() << "]: ";

        // Get qs's edges
        std::vector<std::string> formulas = qs->getOutgoingFormulas();
        // Record all transitions from current state
        std::vector<std::pair<apta_node*, SafetyDFANode*>> transFromCurrQ;

        // for all ql's edges
        for (std::map<int, apta_guard*>::iterator it=ql->guards.begin(); it!=ql->guards.end(); ++it) {
            apta_node* original_next_ql = it->second->target;
            if (original_next_ql == 0) continue;

            int symbolId = it->first;
            std::string symbol =  aut->alphabet[symbolId];

            std::vector<bool> valids;

            for (std::string formula : formulas) {
                bool valid = satisfyFormula(formula, symbol, alphabet);
                valids.push_back(valid);

                if (!valid) continue;

                // Get "original" next state from APTA tree
                // But get the "survived" state that swallowed
                // the original state
                apta_node* next_ql = original_next_ql->find();
                SafetyDFANode* next_qs = qs->getOutgoingNode(formula);

                // Left 2 formulas represent whether to create a loop from left -> left
                // The last formula checks if there is a loop in safety DFA or not
                // if ( (ql == left) && (next_ql == right) && (next_qs != qs) ) {
                //     return false;
                // }

                transFromCurrQ.push_back(std::pair<apta_node*, SafetyDFANode*>(next_ql, next_qs));

                if (!_checkIfVisitedPair(next_ql, next_qs, visited)) {
                    APTAEdge aptaEdge = {ql, symbol, next_ql};
                    SafetyDFAEdge dfaEdge = {qs, formula, next_qs};
                    Q_live.push(aptaEdge);
                    Q_safe.push(dfaEdge);
                    visited.insert(std::pair<apta_node*, SafetyDFANode*>(next_ql, next_qs));
                }
            }

            if (_noValidTransitions(valids)) {
                return false;
            }
        }
    }

    return true;
}


void SafetyDFA::_throwErrorIfNodeNotInDFA(std::string nodeName) {
    // Any node in edges must be defined in the list of nodes
    if (nodeMap.find(nodeName) == nodeMap.end() ) {
        throw "Node [" + nodeName + "] is not defined in nodes";
    }
}


bool SafetyDFA::_checkIfVisitedPair(apta_node*  ql, SafetyDFANode* qs,
                               std::set<std::pair<apta_node*, SafetyDFANode*>> visited) {
    std::pair<apta_node*, SafetyDFANode*> combinedState = std::make_pair(ql, qs);

    std::set<std::pair<apta_node*, SafetyDFANode*>>::iterator it;
    it = visited.find(combinedState);

    // Found
    if (it != visited.end()) return true;

    return false;
}


bool SafetyDFA::_noValidTransitions(std::vector<bool> valids) {
    if (valids.empty()) return true;
    // If all booleans in valids are false
    // Then, there is no more transition
    if (std::all_of(valids.begin(), valids.end(), [](bool i){ return i==false; })) return true;

    return false;
}


bool SafetyDFA::_nextAPTANodeConsistent(std::vector<std::pair<apta_node*, SafetyDFANode*>> transitions) {
    std::set<apta_node*> uniqueNodes;
    std::map<apta_node*, std::vector<SafetyDFANode*>> dupPosMap;

    // First Find those who have same next APTA Node
    for (const std::pair<apta_node*, SafetyDFANode*> &t : transitions) {

        // If the node is unique in the set
        if (uniqueNodes.insert(t.first).second) {
            std::vector<SafetyDFANode*> nextSafetyNodes;
            nextSafetyNodes.push_back(t.second);
            dupPosMap.insert(std::make_pair(t.first, nextSafetyNodes));
        // If there are duplicates
        } else {
            auto it = dupPosMap.find(t.first);
            if (it !=  dupPosMap.end()) {
                std::vector<SafetyDFANode*> nextSafetyNodes = it->second;
                nextSafetyNodes.push_back(t.second);
                it->second = nextSafetyNodes;
            }
        }
    }

    // Now we need to check if next SafetyDFANodes are also same
    for (std::map<apta_node*, std::vector<SafetyDFANode*>>::iterator it=dupPosMap.begin(); it!=dupPosMap.end(); ++it) {
        apta_node* keyNode = it->first;
        std::vector<SafetyDFANode*> v = it->second;

        // If there is a different transition, then return false
        if ( !std::equal(v.begin() + 1, v.end(), v.begin()) ) {
            return false;
        }
    }

    return true;
}


std::set<std::string> SafetyDFA::_extractSymbols(apta*  aut){
    std::set<std::string> symbols;

    for (std::map<int, std::string>::iterator it=aut->alphabet.begin();
         it!=aut->alphabet.end(); ++it) {
        symbols.insert(it->second);
    }

    return symbols;
}

