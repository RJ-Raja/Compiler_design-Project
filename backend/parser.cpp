#include "parser.h"
#include <algorithm>
#include <cctype>

// --- HELPER FUNCTIONS FOR CASE INSENSITIVITY AND OPTIONAL PERIODS ---
static std::string toLower(const std::string& str) {
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
        [](unsigned char c){ return std::tolower(c); });
    return lowerStr;
}

static bool isStatementStart(const std::string& val, TokenType type) {
    std::string lower = toLower(val);
    return (type == TOKEN_CREATE || lower == "create" ||
            type == TOKEN_SET || lower == "set" ||
            type == TOKEN_PRINT || lower == "print" ||
            type == TOKEN_IF || lower == "if" ||
            type == TOKEN_REPEAT || lower == "repeat");
}

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), position(0) {}

Token Parser::currentToken() {
    if (position >= tokens.size()) return {TOKEN_EOF, ""};
    return tokens[position];
}

void Parser::advance() {
    if (position < tokens.size()) position++;
}

// Crucial: This enforces your strict grammar rules
void Parser::expect(TokenType expectedType, const std::string& errorMessage) {
    if (currentToken().type == expectedType) {
        advance();
    } else {
        throw std::runtime_error("Syntax Error: " + errorMessage + " Found: " + currentToken().value);
    }
}

std::unique_ptr<ProgramNode> Parser::parseProgram() {
    auto program = std::make_unique<ProgramNode>();
    
    // Keep parsing statements until we hit the end of the file
    while (currentToken().type != TOKEN_EOF) {
        if (currentToken().type == TOKEN_PERIOD) {
            advance(); // Skip loose/optional periods safely
            continue;
        }
        program->statements.push_back(parseStatement());
    }
    return program;
}

std::unique_ptr<ASTNode> Parser::parseStatement() {
    Token token = currentToken();
    std::string val = toLower(token.value);
    
    if (token.type == TOKEN_CREATE || val == "create") {
        return parseVariableDeclaration();
    } else if (token.type == TOKEN_SET || val == "set") {
        return parseAssignment();
    } else if (token.type == TOKEN_PRINT || val == "print") {
        return parsePrint();
    } else if (token.type == TOKEN_IF || val == "if") {          
        return parseIfStatement();
    } else if (token.type == TOKEN_REPEAT || val == "repeat") {      
        return parseRepeatStatement();
    } else {
        throw std::runtime_error("Syntax Error: Unknown command starting with '" + token.value + "'");
    }
}

// Rule: "Create integer [identifier]."
// OR: "Create [identifier] with value [something]."
std::unique_ptr<ASTNode> Parser::parseVariableDeclaration() {
    auto node = std::make_unique<VarDeclNode>();
    
    advance(); // Consume 'create'
    
    Token next = currentToken();
    bool isNewSyntax = false;
    
    // Check if it's the new syntax: Create [identifier] with value ...
    if (next.type == TOKEN_IDENTIFIER) {
        if (position + 1 < tokens.size() && toLower(tokens[position + 1].value) == "with") {
            isNewSyntax = true;
        }
    }
    
    if (isNewSyntax) {
        node->varName = currentToken().value; // Keep variable names exactly as typed (case sensitive)
        advance(); // consume identifier
        
        advance(); // consume 'with'
        
        if (toLower(currentToken().value) == "value") advance(); 
        else throw std::runtime_error("Expected 'value'");
        
        std::string valueStr = "";
        while (position < tokens.size()) {
            if (currentToken().type == TOKEN_PERIOD) {
                // Recombine decimal points for floats (e.g., 3 . 14)
                if (position + 1 < tokens.size() && tokens[position + 1].type == TOKEN_NUMBER) {
                    valueStr += ".";
                    advance();
                    continue;
                } else {
                    advance(); // Consume the period
                    break; 
                }
            }
            
            // If no period is used, stop when we see the start of the NEXT command
            if (isStatementStart(currentToken().value, currentToken().type)) {
                break;
            }
            
            if (!valueStr.empty() && currentToken().type == TOKEN_IDENTIFIER) valueStr += " ";
            valueStr += currentToken().value;
            advance();
        }
        
        // Auto-Infer Type
        std::string cppType = "auto";
        std::string lowerValueStr = toLower(valueStr);
        if (lowerValueStr == "true" || lowerValueStr == "false") {
            cppType = "bool";
            valueStr = lowerValueStr; // C++ strictly expects lowercase true/false
        } else if (valueStr.front() == '\'' && valueStr.back() == '\'') {
            cppType = "char";
        } else if (valueStr.front() == '"' && valueStr.back() == '"') {
            cppType = "std::string";
        } else if (valueStr.find('.') != std::string::npos) {
            cppType = "float";
        } else {
            cppType = "int"; 
        }
        
        // If it's a raw word/sentence without quotes, turn it into a std::string automatically!
        bool isNumber = true;
        for (char c : valueStr) {
            if (!isdigit(c) && c != '.') {
                isNumber = false;
                break;
            }
        }
        if (!isNumber && cppType != "bool" && cppType != "char" && cppType != "std::string") {
            cppType = "std::string";
            valueStr = "\"" + valueStr + "\"";
        }
        
        node->type = cppType;
        node->initialValue = valueStr;
        node->hasInitialValue = true;
        
        return node;
    }
    
    // Fallback to older syntax (Create integer x.)
    std::string typeVal = toLower(next.value);
    std::string cppType = "";
    
    if (next.type == TOKEN_INTEGER || typeVal == "integer" || typeVal == "int") {
        cppType = "int";
        advance();
    } else if (typeVal == "float") {
        cppType = "float";
        advance();
    } else if (typeVal == "char" || typeVal == "character") {
        cppType = "char";
        advance();
    } else if (typeVal == "bool" || typeVal == "boolean") {
        cppType = "bool";
        advance();
    } else {
        throw std::runtime_error("Syntax Error: Expected 'integer', 'float', 'char', or 'bool'. Found: " + typeVal);
    }
    
    node->type = cppType;
    node->varName = currentToken().value;
    node->hasInitialValue = false;
    
    if (currentToken().type == TOKEN_IDENTIFIER) advance();
    else throw std::runtime_error("Expected a variable name");
    
    return node;
}

// Rule: "Set [identifier] to [value]"
std::unique_ptr<ASTNode> Parser::parseAssignment() {
    auto node = std::make_unique<AssignmentNode>();
    
    advance(); // Consume 'set'
    
    node->varName = currentToken().value;
    expect(TOKEN_IDENTIFIER, "Expected a variable name");
    
    if (toLower(currentToken().value) == "to" || currentToken().type == TOKEN_TO) advance();
    else throw std::runtime_error("Expected 'to'");
    
    std::string valueStr = "";
    while (position < tokens.size()) {
        if (currentToken().type == TOKEN_PERIOD) {
            if (position + 1 < tokens.size() && tokens[position + 1].type == TOKEN_NUMBER) {
                valueStr += ".";
                advance();
                continue;
            } else {
                advance(); // consume period
                break; 
            }
        }
        
        if (isStatementStart(currentToken().value, currentToken().type)) {
            break;
        }
        
        if (!valueStr.empty() && currentToken().type == TOKEN_IDENTIFIER) valueStr += " ";
        valueStr += currentToken().value;
        advance();
    }
    
    bool isNumber = true;
    for (char c : valueStr) {
        if (!isdigit(c) && c != '.') {
            isNumber = false;
            break;
        }
    }
    
    std::string lowerValueStr = toLower(valueStr);
    if (!isNumber && lowerValueStr != "true" && lowerValueStr != "false" && 
        !(valueStr.front() == '\'' && valueStr.back() == '\'') && 
        !(valueStr.front() == '"' && valueStr.back() == '"')) {
        valueStr = "\"" + valueStr + "\"";
    } else if (lowerValueStr == "true" || lowerValueStr == "false") {
        valueStr = lowerValueStr; // normalize boolean case for C++
    }
    
    node->value = valueStr;
    
    return node;
}

// Rule: "Print [identifier]"
std::unique_ptr<ASTNode> Parser::parsePrint() {
    auto node = std::make_unique<PrintNode>();
    
    advance(); // consume 'print'
    
    node->varName = currentToken().value;
    expect(TOKEN_IDENTIFIER, "Expected a variable name to print");
    
    return node;
}

// Rule: "If [identifier] is greater than [number] then [action]"
std::unique_ptr<ASTNode> Parser::parseIfStatement() {
    auto node = std::make_unique<IfNode>();
    
    advance(); // consume 'if'
    
    node->conditionVar = currentToken().value;
    expect(TOKEN_IDENTIFIER, "Expected a variable name");
    
    if (toLower(currentToken().value) == "is" || currentToken().type == TOKEN_IS) advance();
    else throw std::runtime_error("Expected 'is'");
    
    if (toLower(currentToken().value) == "greater" || currentToken().type == TOKEN_GREATER) advance();
    else throw std::runtime_error("Expected 'greater'");
    
    if (toLower(currentToken().value) == "than" || currentToken().type == TOKEN_THAN) advance();
    else throw std::runtime_error("Expected 'than'");
    
    node->conditionValue = std::stoi(currentToken().value);
    expect(TOKEN_NUMBER, "Expected a number");
    
    if (toLower(currentToken().value) == "then" || currentToken().type == TOKEN_THEN) advance();
    else throw std::runtime_error("Expected 'then'");
    
    // Recursively parse the action (e.g., "Print x.")
    node->action = parseStatement(); 
    
    return node;
}

// Rule: "Repeat [number] times: [action]"
std::unique_ptr<ASTNode> Parser::parseRepeatStatement() {
    auto node = std::make_unique<RepeatNode>();
    
    advance(); // consume 'repeat'
    
    node->count = std::stoi(currentToken().value);
    expect(TOKEN_NUMBER, "Expected a number");
    
    if (toLower(currentToken().value) == "times" || currentToken().type == TOKEN_TIMES) advance();
    else throw std::runtime_error("Expected 'times'");
    
    if (currentToken().type == TOKEN_COLON || currentToken().value == ":") advance();
    else throw std::runtime_error("Expected ':'");
    
    // Recursively parse the action inside the loop (e.g., "Print x.")
    node->action = parseStatement();
    
    return node;
}