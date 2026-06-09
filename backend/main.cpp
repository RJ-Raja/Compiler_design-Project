#include <iostream>
#include <fstream>
#include <sstream>
#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "codegen.h"

int main(int argc, char* argv[]) {
    // Use command-line arguments for file paths if provided
    std::string inputFilePath = (argc > 1) ? argv[1] : "IO/test_input.txt";
    std::string outputFilePath = (argc > 2) ? argv[2] : "IO/output.cpp";

    std::cout << "\nStarting Team RS C++ Compiler...\n\n";

    // --- PHASE 1: READ INPUT ---
    std::ifstream file(inputFilePath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open " << inputFilePath << std::endl;
        return 1;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    
    std::cout << "--- RAW ENGLISH INPUT ---\n";
    std::cout << source << "\n";
    std::cout << "-------------------------\n\n";

    try {
        // --- PHASE 2: LEXICAL ANALYSIS ---
        std::cout << ">>> Running Lexical Analyzer...\n";
        Lexer lexer(source);
        std::vector<Token> tokens = lexer.tokenize();
        
        std::cout << "=== TOKEN STREAM ===\n";
        for (const auto& token : tokens) {
            std::cout << "[Type: " << token.type << ", Value: '" << token.value << "'] ";
        }
        std::cout << "\n====================\n\n";


        // --- PHASE 3: PARSING (SYNTAX ANALYSIS) ---
        std::cout << ">>> Running Parser...\n";
        Parser parser(tokens);
        std::unique_ptr<ProgramNode> ast = parser.parseProgram();
        
        // Print the newly created tree visualization!
        ast->print(0); 
        std::cout << "\n";


 // --- PHASE 4/5: SEMANTIC ANALYSIS (LOGIC CHECK) ---
        std::cout << ">>> Running Semantic Analyzer...\n";
        SemanticAnalyzer analyzer;
        analyzer.analyze(ast.get());
        std::cout << "Semantic Analysis: Success! No logical errors found.\n\n";

        // --- PHASE 6: CODE GENERATOR ---
        std::cout << ">>> Running C++ Code Generator...\n";
        CodeGenerator generator;
        
        generator.generate(ast.get(), outputFilePath); 
        
        std::cout << "Code Generation: Success! File has been created at: " << outputFilePath << "\n\n";
        std::cout << "=== COMPILATION PIPELINE FINISHED SUCCESSFULLY ===\n";

    } catch (const std::exception& e) {
        std::cerr << "\nCOMPILATION FAILED: " << e.what() << "\n"; 
        return 1; // Returning non-zero exit code so the server knows compilation failed
    }

    return 0;
}