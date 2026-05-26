#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>

// Helper function for indentation
inline void printIndent(int indent) {
    for (int i = 0; i < indent; i++) std::cout << "  ";
}

// Base class
struct ASTNode {
    virtual ~ASTNode() = default;
    virtual void print(int indent = 0) const = 0; // The new print rule
};

// Node for "Create integer x."
struct VarDeclNode : public ASTNode {
    std::string varName;
    std::string type; 
    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "[VarDeclNode] Create " << type << " -> " << varName << "\n";
    }
};

// Node for "Set x to 10."
struct AssignmentNode : public ASTNode {
    std::string varName;
    int value;
    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "[AssignmentNode] Set " << varName << " = " << value << "\n";
    }
};

// Node for "Print x."
struct PrintNode : public ASTNode {
    std::string varName;
    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "[PrintNode] Print -> " << varName << "\n";
    }
};

// Node for "If x is greater than 5 then Print x."
struct IfNode : public ASTNode {
    std::string conditionVar;
    int conditionValue;
    std::unique_ptr<ASTNode> action;
    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "[IfNode] IF " << conditionVar << " > " << conditionValue << " THEN:\n";
        if (action) action->print(indent + 2); // Indent the action further!
    }
};

// Node for "Repeat 3 times: Print x."
struct RepeatNode : public ASTNode {
    int count;
    std::unique_ptr<ASTNode> action;
    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "[RepeatNode] REPEAT " << count << " TIMES:\n";
        if (action) action->print(indent + 2); // Indent the action further!
    }
};

// The root of the program
struct ProgramNode : public ASTNode {
    std::vector<std::unique_ptr<ASTNode>> statements;
    void print(int indent = 0) const override {
        std::cout << "=== ABSTRACT SYNTAX TREE ===\n";
        for (const auto& stmt : statements) {
            stmt->print(indent);
        }
        std::cout << "============================\n";
    }
};

#endif