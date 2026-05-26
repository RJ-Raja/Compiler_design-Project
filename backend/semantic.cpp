#include "semantic.h"

void SemanticAnalyzer::analyze(const ProgramNode* program) {
    // Loop through every statement in the main program tree
    for (const auto& stmt : program->statements) {
        analyzeNode(stmt.get());
    }
}

void SemanticAnalyzer::analyzeNode(const ASTNode* node) {
    if (!node) return;

    // Rule 1: Creating a variable
    if (auto v = dynamic_cast<const VarDeclNode*>(node)) {
        // Check if it already exists in our memory
        if (symbolTable.find(v->varName) != symbolTable.end()) {
            throw std::runtime_error("Semantic Error: Variable '" + v->varName + "' is already declared.");
        }
        // If it's safe, add it to the Symbol Table
        symbolTable[v->varName] = v->type;
    }
    
    // Rule 2: Assigning a value to a variable
    else if (auto a = dynamic_cast<const AssignmentNode*>(node)) {
        if (symbolTable.find(a->varName) == symbolTable.end()) {
            throw std::runtime_error("Semantic Error: Variable '" + a->varName + "' was not declared before assignment.");
        }
    }
    
    // Rule 3: Printing a variable
    else if (auto p = dynamic_cast<const PrintNode*>(node)) {
        if (symbolTable.find(p->varName) == symbolTable.end()) {
            throw std::runtime_error("Semantic Error: Cannot print undeclared variable '" + p->varName + "'.");
        }
    }
    
    // Rule 4: If Statements
    else if (auto i = dynamic_cast<const IfNode*>(node)) {
        if (symbolTable.find(i->conditionVar) == symbolTable.end()) {
            throw std::runtime_error("Semantic Error: Variable '" + i->conditionVar + "' used in IF condition is not declared.");
        }
        // Recursively check the action inside the IF block!
        analyzeNode(i->action.get());
    }
    
    // Rule 5: Repeat Loops
    else if (auto r = dynamic_cast<const RepeatNode*>(node)) {
        // Recursively check the action inside the loop!
        analyzeNode(r->action.get());
    }
    
}


void SemanticAnalyzer::printSymbolTable() const {
    std::cout << "=== SYMBOL TABLE STATE ===\n";
    if (symbolTable.empty()) {
        std::cout << "  (Empty - No variables tracked yet)\n";
    } else {
        for (const auto& pair : symbolTable) {
            std::cout << "  Name: [" << pair.first << "] | Type: [" << pair.second << "]\n";
        }
    }
    std::cout << "==========================\n";
}