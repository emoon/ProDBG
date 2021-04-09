// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaComponent.h"
#include "Interpreter.h"

class RetroShell : public AmigaComponent {

    // Interpreter for commands typed into the console window
    Interpreter interpreter;
    
    //
    // Text storage
    //
    
    // The text storage
    std::vector<string> storage;
    string all;
    
    // The input history buffer
    std::vector<string> input;

    // Input prompt
    string prompt = "vAmiga\% ";
    
    // The current cursor position
    isize cpos = 0;

    // The minimum cursor position in this row
    isize cposMin = 0;
    
    // The currently active input string
    isize ipos = 0;

    // Indicates if TAB was the most recently pressed key
    bool tabPressed = false;
    
    bool isDirty = false;
    
    
    //
    // Initializing
    //
    
public:
    
    RetroShell(Amiga& ref);
        
    const char *getDescription() const override { return "RetroShell"; }

    void _reset(bool hard) override { }
    
    
    //
    // Serializing
    //

private:

    isize _size() override { return 0; }
    isize _load(const u8 *buffer) override {return 0; }
    isize _save(u8 *buffer) override { return 0; }

    
    //
    // Managing user input
    //

public:
    
    void pressUp();
    void pressDown();
    void pressLeft();
    void pressRight();
    void pressHome();
    void pressEnd();
    void pressTab();
    void pressBackspace();
    void pressDelete();
    void pressReturn();
    void pressKey(char c);


    //
    // Working with the text storage
    //

public:
    
    const char *text();
    
    // Returns a reference to the text storage
    const std::vector<string> &getStorage() { return storage; }
    
    // Returns the cursor position (relative to the line end)
    isize cposAbs() { return cpos; }
    isize cposRel();

    // Prints a message
    RetroShell &operator<<(char value);
    RetroShell &operator<<(const string &value);
    RetroShell &operator<<(int value);
    RetroShell &operator<<(long value);

    // Moves the cursor forward to a certain column
    void tab(isize hpos);

    // Prints the input prompt
    void printPrompt();

private:

    // Returns a reference to the last line in the text storage
    string &lastLine() { return storage.back(); }
    
    // Returns a reference to the last line in the input history buffer
    string &lastInput() { return input.back(); }

    // Clears the console window
    void clear();
    
    // Prints a help line
    void printHelp();
    
    // Shortens the text storage if it grows too large
    void shorten();
    
    // Clears the current line
    void clearLine() { *this << '\r'; }
    
    
    //
    // Executing commands
    //
    
public:
    
    // Executes a user command
    bool exec(const string &command, bool verbose = false);
    
    // Executes a user script
    void exec(std::istream &stream) throws;
    
    
    //
    // Command handlers
    //
    
public:
    
    // void handler(const string& command) throws;
    
    template <Token t1> void exec(Arguments& argv, long param) throws;
    template <Token t1, Token t2> void exec(Arguments& argv, long param) throws;
    template <Token t1, Token t2, Token t3> void exec(Arguments& argv, long param) throws;

private:
    
    void dump(HardwareComponent &component, Dump::Category category);

};
