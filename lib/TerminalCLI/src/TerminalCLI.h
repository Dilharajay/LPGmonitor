#ifndef TERMINAL_CLI_H
#define TERMINAL_CLI_H

#include <Arduino.h>
#include <functional>
#include "Config.h"

typedef std::function<void(String)> CommandHandler;

struct Command {
    String name;
    String description;
    CommandHandler handler;
};

class TerminalCLI {
public:
    TerminalCLI();
    void begin(const String& prompt = "Terminal Ready.");
    void handle();
    
    // Register a command. If aliases are needed, register them separately.
    void registerCommand(const String& name, const String& description, CommandHandler handler);
    void printHelp();

private:
    Command commands[Config::MAX_CLI_COMMANDS];
    int commandCount;
    String serialInputBuffer;
};

#endif
