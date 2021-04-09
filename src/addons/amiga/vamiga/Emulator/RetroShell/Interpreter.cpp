// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Interpreter.h"
#include "RetroShell.h"
#include <sstream>

Interpreter::Interpreter(Amiga &ref) : AmigaComponent(ref)
{
    registerInstructions();
};

Arguments
Interpreter::split(const string& userInput)
{
    std::stringstream ss(userInput);
    Arguments result;

    string token;
    bool str = false;
    
    for (usize i = 0; i < userInput.size(); i++) {

        // Switch between string mode and non-string mode if '"' is detected
        if (userInput[i] == '"') { str = !str; continue; }
        
        // Process character
        if (userInput[i] != ' ' || str) {
            token += userInput[i];
        } else {
            if (!token.empty()) result.push_back(token);
            token = "";
        }
    }
    if (!token.empty()) result.push_back(token);
    
    /*
    printf("Tokens:\n");
    for (auto &it : result) {
        printf("%s\n", it.c_str());
    }
    */
    return result;
}
    
void
Interpreter::autoComplete(Arguments &argv)
{
    Command *current = &root;
    string prefix, token;

    for (auto it = argv.begin(); current && it != argv.end(); it++) {
        
        *it = current->autoComplete(*it);
        current = current->seek(*it);
    }
}

string
Interpreter::autoComplete(const string& userInput)
{
    string result;
    
    // Split input string
    Arguments tokens = split(userInput);
    
    // Complete all tokens
    autoComplete(tokens);

    // Recreate the command string
    for (const auto &it : tokens) {
        result += (result == "" ? "" : " ") + it;
    }

    // Add a space if the command has been fully completed
    printf("autoComplete: '%s'\n", result.c_str());
    if (root.seek(tokens) != nullptr) {
        printf("Adding space\n");
        result += " ";
    }
    
    return result;
}

void
Interpreter::exec(const string& userInput, bool verbose)
{
    // Split the command string
    Arguments tokens = split(userInput);
        
    // Auto complete the token list
    autoComplete(tokens);
            
    // Process the command
    exec(tokens, verbose);
}

void
Interpreter::exec(Arguments &argv, bool verbose)
{
    Command *current = &root;
    string token;

    // In 'verbose' mode, print the token list
    if (verbose) {
        for (const auto &it : argv) retroShell << it << ' ';
        retroShell << '\n';
    }
    
    // Skip empty lines
    if (argv.empty()) return;
    
    // Seek the command in the command tree
    while (current) {
        
        // Extract token
        token = argv.empty() ? "" : argv.front();
        
        // Break the loop if this token is unknown
        Command *next = current->seek(token);
        if (next == nullptr) break;
        
        // Move one level down
        current = next;
        if (!argv.empty()) argv.pop_front();
    }
        
    // Error out if no command handler is present
    if (current->action == nullptr && !argv.empty()) {
        printf("NO COMMAND HANDLER FOUND\n");
        throw util::ParseError(token);
    }
    if (current->action == nullptr && argv.empty()) {
        throw TooFewArgumentsError(current->tokens());
    }
    
    // Check the argument count
    if ((isize)argv.size() < current->numArgs) throw TooFewArgumentsError(current->tokens());
    if ((isize)argv.size() > current->numArgs) throw TooManyArgumentsError(current->tokens());
    
    // Call the command handler
    printf("TODO:CALL COMMAND HANDLER FOR command object %p\n", current);
    (retroShell.*(current->action))(argv, current->param);
}

void
Interpreter::usage(Command& current)
{
    retroShell << "Usage: " << current.usage() << '\n' << '\n';
}

void
Interpreter::help(const string& userInput)
{
    printf("help(%s)\n", userInput.c_str());
    
    // Split the command string
    Arguments tokens = split(userInput);
        
    // Auto complete the token list
    autoComplete(tokens);
            
    // Process the command
    help(tokens);
}

void
Interpreter::help(Arguments &argv)
{
    Command *current = &root;
    string prefix, token;
    
    retroShell << '\n';
    
    while (1) {
                
        // Extract token
        token = argv.empty() ? "" : argv.front();
        
        // Check if this token matches a known command
        Command *next = current->seek(token);
        if (next == nullptr) break;
        
        prefix += next->token + " ";
        current = next;
        if (!argv.empty()) argv.pop_front();
    }

    help(*current);
}

void
Interpreter::help(Command& current)
{
    // Print the usage string
    usage(current);
    
    // Collect all argument types
    auto types = current.types();

    // Determine tabular positions to align the output
    int tab = 0;
    // for (auto &it : types) tab = std::max(tab, (int)it.length());
    for (auto &it : current.args) {
        tab = std::max(tab, (int)it.token.length());
        tab = std::max(tab, 2 + (int)it.type.length());
    }
    tab += 5;
    
    for (auto &it : types) {
        
        auto opts = current.filterType(it);
        int size = (int)it.length();

        retroShell.tab(tab - size);
        retroShell << "<" << it << "> : ";
        retroShell << (int)opts.size() << (opts.size() == 1 ? " choice" : " choices");
        retroShell << '\n' << '\n';
        
        for (auto &opt : opts) {

            string name = opt->token == "" ? "<>" : opt->token;
            retroShell.tab(tab + 2 - (int)name.length());
            retroShell << name;
            retroShell << " : ";
            retroShell << opt->info;
            retroShell << '\n';
        }
        retroShell << '\n';
    }
}
