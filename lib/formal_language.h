#ifndef _FORMAL_LANGUAGE_H_
#define _FORMAL_LANGUAGE_H_

#include <iostream>
#include <fstream>
#include <map>
#include <vector>


bool andFunc(bool a, bool b) {
    return a && b;
}


bool orFunc(bool a, bool b) {
    return a || b;
}


bool isStringFound(std::string::size_type position) {
    return (position != std::string::npos);
}


void separateFormula(std::string formula,
                     std::vector<std::string>& usedSymbols,
                     std::vector<std::string>& booleanOps,
                     std::vector<std::string> operators,
                     std::string replacement = " * ") {

    for (std::string op : operators) {
        if (op.size() != replacement.size()) {
            throw "Size of the operators and replacements must be same";
        }
    }

    std::vector<std::string::size_type> positions;
    std::map<std::string::size_type, std::string> operatorPositions;

    // TODO: Make the inner operation a for loop
    // so that it can accept more operators
    while (true) {
        std::vector<bool> founds;

        for (std::string op : operators) {
            // Find String
            std::string::size_type pos = formula.find(op);
            bool found = isStringFound(pos);
            founds.push_back(found);
            // If Found, Save the operator's start position
            if (found) {
                formula.replace(pos, op.length(), replacement);
                positions.push_back(pos);
                operatorPositions.insert(std::pair<std::string::size_type, std::string>(pos, op));
            }
        }

        // If nothing was found, then break the while loop
        if (std::all_of(founds.begin(), founds.end(), [](bool i){ return i==false; })) break;
    }


    // Sort minimum first
    std::sort(positions.begin(), positions.end());

    /*
     * Now Separate Symbols and Boolean Formulas
     **/
    std::string symbol;

    // From the head to the first operator
    if (positions.size()==0) {
        symbol = formula.substr(0, formula.size());
        usedSymbols.push_back(symbol);
    } else {
        int p = static_cast<int>(positions[0]);
        symbol = formula.substr(0, p);
        usedSymbols.push_back(symbol);
    }

    for (int i=0; i<positions.size(); i++) {
        // Get a position of an operator
        std::string::size_type position = positions[i];
        int p = static_cast<int>(position);

        // Append Boolean Operators to a list in order
        std::string op = operatorPositions[position];
        int opSize = static_cast<int>(op.size());
        int start = p + opSize;
        booleanOps.push_back(op);

        // From the last operator until formula's end
        if (i==positions.size()-1) {
            int n = formula.size() - start;
            symbol = formula.substr(start, n);
        // Operators in between
        } else {
            std::string::size_type nextPosition = positions[i+1];
            int nextP = static_cast<int>(nextPosition);
            int n = nextP - start;
            symbol = formula.substr(start, n);
        }

        usedSymbols.push_back(symbol);
    }
}


/**
 * @brief Evalute a symbol (which may contain !) given boolean symbols
 **/
bool evaluateSymbol(
    std::string symbol,
    std::map<std::string, bool> booleanSymbols) {

    std::string::size_type pos = symbol.find("!");
    // If ! is not included, then return true
    // If found, then return false
    bool found = isStringFound(pos);

    // If found, remove ! from the string
    if (found) {
        symbol = symbol.substr(1, symbol.size()-1);
    }

    bool booleanSymbol = booleanSymbols[symbol];

    // Flip the sign if contains !
    if (found) {
        return !booleanSymbol;
    }

    return booleanSymbol;
}


/**
 * @brief Check whether sigma satisfies formula
 **/
bool satisfyFormula(std::string formula,
                    std::string sigma,
                    std::set<std::string> symbols,
                    OperatorMap operatorMap = {}){

    if (operatorMap.empty()) {
        operatorMap[" & "] = andFunc;
        operatorMap[" | "] = orFunc;
    }

    std::vector<std::string> booleanOperators;
    for (OperatorMap::iterator it=operatorMap.begin(); it!=operatorMap.end(); ++it) {
        booleanOperators.push_back(it->first);
    }

    std::map<std::string, bool> booleanSymbols;
    for (std::string symbol : symbols) {
        booleanSymbols.insert(std::pair<std::string, bool>(symbol, false));
    }

    booleanSymbols[sigma] = true;

    // TODO: Separate boolean_formula into a list of symbols
    //       and a list of boolean operators
    std::vector<std::string> usedSymbols;
    std::vector<std::string> booleanOps;

    separateFormula(formula, usedSymbols, booleanOps, booleanOperators);


    if (usedSymbols.size() == 0) {
        throw "Could not find any symbol in the formula: " + formula;
    }

    if (usedSymbols.size() - booleanOps.size() != 1) {
        for (std::string s : usedSymbols) std::cerr << s << std::endl;
        for (std::string b : booleanOps) std::cerr << b << std::endl;
        throw "Could not find the right No. of Symbols and Boolean Operators";
    }

    // TODO: Convert all symbols to a boolean

    bool currEval = evaluateSymbol(usedSymbols[0], booleanSymbols);
    for (int i=0; i<booleanOps.size(); i++) {
        std::string op = booleanOps[i];
        bool e = evaluateSymbol(usedSymbols[i+1], booleanSymbols);

        currEval = operatorMap[op](currEval, e);
    }

    return currEval;
}



#endif /* _FORMAL_LANGUAGE_H_ */
