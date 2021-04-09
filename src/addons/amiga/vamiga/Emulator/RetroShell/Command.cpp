// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Command.h"
#include <algorithm>

Command *
Command::add(std::vector<string> tokens,
             const string &a1,
             const string &help,
             void (RetroShell::*action)(Arguments&, long),
             isize numArgs, long param)
{
    assert(!tokens.empty());
     
    if (tokens.size() > 1) {

        auto node = seek(tokens.front());
        tokens.erase(tokens.begin());
        return node->add(tokens, a1, help, action, numArgs, param);
    }
        
    // Register instruction
    Command d { this, tokens.front(), a1, help, std::list<Command>(), action, numArgs, param };
    args.push_back(d);
    return seek(tokens.front());
}

Command *
Command::add(std::vector<string> firstTokens,
             std::vector<string> tokens,
             const string &a1,
             const string &help,
             void (RetroShell::*action)(Arguments&, long),
             isize numArgs, long param)
{
    for (usize i = 0; i < firstTokens.size(); i++) {
        
        tokens[0] = firstTokens[i];
        add(tokens, a1, help, action, numArgs, i);
    }
    return nullptr;
}

void
Command::remove(const string& token)
{
    for(auto it = std::begin(args); it != std::end(args); ++it) {
        if (it->token == token) { args.erase(it); return; }
    }
}

Command *
Command::seek(const string& token)
{
    for (auto& it : args) {
        if (it.token == token) return &it;
    }
    return nullptr;
}

Command *
Command::seek(Arguments argv)
{
    Command *result = nullptr;
    
    if (!argv.empty()) {
        result = this;
        for (auto& it : argv) {
            if (!(result = result->seek(it))) break;
        }
    }
    
    return result;
}

std::vector<string>
Command::types()
{
    std::vector<string> result;
    
    for (auto &it : args) {
        
        if (it.hidden) continue;
        
        if (std::find(result.begin(), result.end(), it.type) == result.end()) {
            result.push_back(it.type);
        }
    }
    
    return result;
}

std::vector<Command *>
Command::filterType(const string& type)
{
    std::vector<Command *> result;
    
    for (auto &it : args) {
        
        if (it.hidden) continue;
        if (it.type == type) result.push_back(&it);
    }
    
    return result;
}
std::vector<Command *>
Command::filterPrefix(const string& prefix)
{
    std::vector<Command *> result;
    
    for (auto &it : args) {
        if (it.hidden) continue;
        if (it.token.substr(0, prefix.size()) == prefix) result.push_back(&it);
    }

    return result;
}

string
Command::autoComplete(const string& token)
{
    string result = token;
    
    auto matches = filterPrefix(token);
    if (!matches.empty()) {
        
        Command *first = matches.front();
        for (usize i = token.size(); i < first->token.size(); i++) {
            
            for (auto m: matches) {
                if (m->token.size() <= i || m->token[i] != first->token[i]) {
                    return result;
                }
            }
            result += first->token[i];
        }
    }
    return result;
}

string
Command::tokens()
{
    string result = this->parent ? this->parent->tokens() : "";
    return result == "" ? token : result + " " + token;
}

string
Command::usage()
{
    string firstArg, otherArgs;
    
    if (args.empty()) {

        firstArg = numArgs == 0 ? "" : numArgs == 1 ? "<value>" : "<values>";

    } else {
        
        // Collect all argument types
        auto t = types();
        
        // Describe the first argument
        for (usize i = 0; i < t.size(); i++) {
            firstArg += (i == 0 ? "" : "|") + t[i];
        }
        firstArg = "<" + firstArg + ">";
        
        // Describe the remaining arguments (if any)
        bool printArg = false, printOpt = false;
        for (auto &it : args) {
            if (it.action != nullptr && it.numArgs == 0) printOpt = true;
            if (it.numArgs > 0 || !it.args.empty()) printArg = true;
        }
        if (printArg) {
            otherArgs = printOpt ? "[<arguments>]" : "<arguments>";
        }
    }
    
    return tokens() + " " + firstArg + " " + otherArgs;
}
