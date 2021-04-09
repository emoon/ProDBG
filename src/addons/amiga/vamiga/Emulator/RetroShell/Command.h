// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"
#include <list>
#include <vector>

class RetroShell;

typedef std::list<string> Arguments;

struct Command {
    
    // Pointer to the parent command
    Command *parent = nullptr;
    
    // The token string (e.g., "agnus" or "set")
    string token;
    
    // A string describing the token type (e.g., "component or "command")
    string type;
    
    // The help string for this command
    string info;
    
    // The sub commands of this command
    std::list<Command> args;
    
    // Command handler
    // std::function <void(Arguments&, long)> action;
    void (RetroShell::*action)(Arguments&, long) = nullptr;
    
    // Number of additional arguments expected by the command handler
    isize numArgs = 0;
    
    // An additional paramter passed to the command handler
    long param = 0;
    
    // Indicates if this command appears in the help descriptions
    bool hidden = false;
        
    // Creates a new node in the command tree
    Command *add(std::vector<string> tokens,
                 const string &a1,
                 const string &help,
                 void (RetroShell::*action)(Arguments&, long) = nullptr,
                 isize numArgs = 0, long param = 0);

    // Creates multiple nodes in the command tree
    Command *add(std::vector<string> firstTokens,
                 std::vector<string> tokens,
                 const string &a1,
                 const string &help,
                 void (RetroShell::*action)(Arguments&, long) = nullptr,
                 isize numArgs = 0, long param = 0);

    // Removes a registered command
    void remove(const string& token);
    
    // Seeks a command object inside the command object tree
    Command *seek(const string& token);
    Command *seek(Arguments argv);
    
    // Collects the type descriptions in the args vector
    std::vector<string> types();
    
    // Filters the argument list
    std::vector<Command *> filterType(const string& type);
    std::vector<Command *> filterPrefix(const string& prefix);

    // Automatically completes a partial token string
    string autoComplete(const string& token);
    
    // Returns the full command string for this command
    string tokens();
    
    // Returns a syntax string for this command
    string usage();
};
