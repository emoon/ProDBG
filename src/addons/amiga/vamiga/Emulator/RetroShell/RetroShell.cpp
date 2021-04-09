// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "RetroShell.h"
#include "Amiga.h"
#include "Parser.h"
#include <sstream>

RetroShell::RetroShell(Amiga& ref) : AmigaComponent(ref), interpreter(ref)
{
    // Initialize the text storage
    storage.push_back("");

    // Initialize the input buffer
    input.push_back("");
    
    // Print a startup message
    *this << "vAmiga " << V_MAJOR << '.' << V_MINOR << '.' << V_SUBMINOR;
    *this << " (" << __DATE__ << " " << __TIME__ << ")" << '\n';
    *this << '\n';
    *this << "Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de" << '\n';
    *this << "Licensed under the GNU General Public License v3" << '\n';
    *this << '\n';

    printHelp();
    *this << '\n';
    printPrompt();
}

isize
RetroShell::cposRel()
{
    isize lineLength = (isize)lastLine().size();
    
    return cpos >= lineLength ? 0 : lineLength - cpos;
}

RetroShell&
RetroShell::operator<<(char value)
{
    if (value == '\n') {

        // Newline (appends an empty line)
        storage.push_back("");
        cpos = cposMin = 0;
        shorten();

    } else if (value == '\r') {

        // Carriage return (clears the current line)
        storage.back() = "";
        
    } else {
        
        // Add a single character
        storage.back() += value;
    }
    
    shorten();
    return *this;
}

RetroShell&
RetroShell::operator<<(const string& text)
{
    storage.back() += text;
    return *this;
}

RetroShell&
RetroShell::operator<<(int value)
{
    *this << std::to_string(value);
    return *this;
}

RetroShell&
RetroShell::operator<<(long value)
{
    *this << std::to_string(value);
    return *this;
}

void
RetroShell::tab(isize hpos)
{
    isize delta = hpos - (int)storage.back().length();
    for (isize i = 0; i < delta; i++) {
        *this << ' ';
    }
}

void
RetroShell::printPrompt()
{
    // Finish the current line (if neccessary)
    if (!lastLine().empty()) *this << '\n';

    // Print the prompt
    *this << prompt;
    cpos = cposMin = prompt.size();
}

void
RetroShell::clear()
{
    storage.clear();
    printPrompt();
}

void
RetroShell::printHelp()
{
    *this << "Press 'TAB' twice for help." << '\n';
}

void
RetroShell::shorten()
{
    while (storage.size() > 600) {
        
        storage.erase(storage.begin());
    }
}

void
RetroShell::pressUp()
{
    if (ipos == (isize)input.size() - 1) {
        lastInput() = lastLine().substr(cposMin);
    }
    
    if (ipos > 0) ipos--;
    if (ipos < (isize)input.size()) lastLine() = prompt + input[ipos];
    tabPressed = false;
}

void
RetroShell::pressDown()
{
    if (ipos + 1 < (isize)input.size()) ipos++;
    if (ipos < (isize)input.size()) lastLine() = prompt + input[ipos];
    tabPressed = false;
}

void
RetroShell::pressLeft()
{
    cpos = std::max(cpos - 1, cposMin);
    tabPressed = false;
}

void
RetroShell::pressRight()
{
    cpos = std::min(cpos + 1, (isize)lastLine().size());
    tabPressed = false;
}

void
RetroShell::pressHome()
{
    cpos = cposMin;
    tabPressed = false;
}

void
RetroShell::pressEnd()
{
    cpos = (isize)lastLine().size();
    tabPressed = false;
}

void
RetroShell::pressTab()
{
    if (tabPressed) {
        
        // TAB was pressed twice
        string currentInput = lastLine();
        isize cposMinOld = cposMin;
        
        // *this << '\n';
        
        // Print the instructions for this command
        interpreter.help(lastLine().substr(cposMin));
        
        // Repeat the old input string
        *this << currentInput;
        cposMin = cposMinOld;
        cpos = lastLine().length();
        
    } else {
        
        // Auto-complete the typed in command
        string stripped = storage.back().substr(cposMin);
        lastLine() = prompt + interpreter.autoComplete(stripped);
        cpos = (isize)lastLine().length();
    }
    
    tabPressed = true;
}

void
RetroShell::pressBackspace()
{
    if (cpos > cposMin) {
        lastLine().erase(lastLine().begin() + --cpos);
    }
    tabPressed = false;
}

void
RetroShell::pressDelete()
{
    if (cpos < (isize)lastLine().size()) {
        lastLine().erase(lastLine().begin() + cpos);
    }
    tabPressed = false;
}

void
RetroShell::pressReturn()
{
    // Get the last line without the prompt
    string command = lastLine().substr(cposMin);
    
    *this << '\n';
    
    // Print help message if there was no input
    if (command.empty()) {
        printHelp();
        printPrompt();
        return;
    }
    
    // Add command to the command history buffer
    input[input.size() - 1] = command;
    input.push_back("");
    ipos = (isize)input.size() - 1;
    
    // Execute the command
    exec(command);
    printPrompt();
    tabPressed = false;
}

void
RetroShell::pressKey(char c)
{    
    if (isprint(c)) {
                
        if (cpos < (isize)lastLine().size()) {
            lastLine().insert(lastLine().begin() + cpos, c);
        } else {
            lastLine() += c;
        }
        cpos++;
        
        isDirty = true;        
        tabPressed = false;
    }
}

const char *
RetroShell::text()
{
    all = "";
    
    if (auto numRows = storage.size()) {
        
        // Add all rows except the last one
        for (usize i = 0; i < numRows - 1; i++) all += storage[i] + "\n";
        
        // Add the last row
        all += storage[numRows - 1] + " ";        
    }
    
    return all.c_str();
}

bool
RetroShell::exec(const string &command, bool verbose)
{
    bool success = false;
    
    // Print the command string if requested
    if (verbose) *this << command << '\n';
        
    printf("Command: %s\n", command.c_str());
 
    try {
        
        // Hand the command over to the intepreter
        interpreter.exec(command);
        success = true;
               
    } catch (TooFewArgumentsError &err) {
        *this << err.what() << ": Too few arguments";
        *this << '\n';
        
    } catch (TooManyArgumentsError &err) {
        *this << err.what() << ": Too many arguments";
        *this << '\n';
            
    } catch (util::EnumParseError &err) {
        *this << err.token << " is not a valid key" << '\n';
        *this << "Expected: " << err.expected << '\n';
        
    } catch (util::ParseNumError &err) {
        *this << err.token << " is not a number" << '\n';

    } catch (util::ParseBoolError &err) {
        *this << err.token << " must be true or false" << '\n';

    } catch (util::ParseError &err) {
        *this << err.what() << ": Syntax error";
        *this << '\n';
        
    } catch (ConfigUnsupportedError &err) {
        *this << "This option is not yet supported.";
        *this << '\n';
        
    } catch (ConfigLockedError &err) {
        *this << "This option is locked because the Amiga is powered on.";
        *this << '\n';
        
    } catch (ConfigArgError &err) {
        *this << "Error: Invalid argument. Expected: " << err.what();
        *this << '\n';
        
    } catch (ConfigFileNotFoundError &err) {
        *this << err.what() << " not found";
        *this << '\n';
        success = true; // Don't break the execution
        
    } catch (ConfigFileReadError &err) {
        *this << "Error: Unable to read file " << err.what();
        *this << '\n';
        
    } catch (VAError &err) {
        *this << err.what();
        *this << '\n';
    }
    
    return success;
}

void
RetroShell::exec(std::istream &stream)
{
    isize line = 0;
    string command;
        
    while(std::getline(stream, command)) {

        line++;
        printf("Line %zd: %s\n", line, command.c_str());

        // Skip empty lines
        if (command == "") continue;

        // Skip comments
        if (command.substr(0,1) == "#") continue;
        
        // Execute the command
        if (!exec(command, true)) {
            throw util::Exception(command, line);
        }
    }
}

/*
bool
RetroShell::parseBool(string& token)
{
    if (token == "1" || token == "true" || token == "yes") return true;
    if (token == "0" || token == "false" || token == "no") return false;

    throw ParseBoolError("");
}

long
RetroShell::parseNum(string& token)
{
    long result;
    
    try { result = stol(token, nullptr, 0); }
    catch (std::exception& err) { throw ParseNumError(token); }

    return result;
}
*/

void
RetroShell::dump(HardwareComponent &component, Dump::Category category)
{
    std::stringstream ss; string line;
    
    amiga.suspend();
    component.dump(category, ss);
    amiga.resume();
    
    while(std::getline(ss, line)) *this << line << '\n';
}
