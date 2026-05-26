#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include <unordered_map>
#include <string>
#include <stdexcept>
#include <iostream> // Added for printing

class SemanticAnalyzer {
public:
    void analyze(const ProgramNode* program);
    
    // NEW: Function to display the tracked variables
    void printSymbolTable() const;

private:
    std::unordered_map<std::string, std::string> symbolTable;
    void analyzeNode(const ASTNode* node);
};

#endif