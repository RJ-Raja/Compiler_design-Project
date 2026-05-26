#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include <fstream>
#include <string>

class CodeGenerator {
public:
    // Takes the AST and the name of the file you want to create
    void generate(const ProgramNode* program, const std::string& outputFilename);

private:
    std::ofstream outFile;
    int currentIndent = 1; // Start at 1 because we are inside int main()

    // Helper functions for formatting
    void writeIndent();
    void generateNode(const ASTNode* node);
};

#endif