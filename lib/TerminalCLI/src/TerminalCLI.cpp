#include "TerminalCLI.h"
#include "Logger.h"

TerminalCLI::TerminalCLI() : commandCount(0), serialInputBuffer("") {}

void TerminalCLI::begin(const String& prompt) {
    if (prompt.length() > 0) {
        Logger::info(prompt);
    }
}

void TerminalCLI::registerCommand(const String& name, const String& description, CommandHandler handler) {
    if (commandCount < Config::MAX_CLI_COMMANDS) {
        commands[commandCount].name = name;
        commands[commandCount].description = description;
        commands[commandCount].handler = handler;
        commandCount++;
    } else {
        Logger::error("Max CLI commands reached!");
    }
}

void TerminalCLI::printHelp() {
    Logger::rawln("\n=== Available Commands ===");
    for (int i = 0; i < commandCount; i++) {
        Logger::raw("  ");
        Logger::raw(commands[i].name);
        // Padding for alignment
        for (int j = commands[i].name.length(); j < 15; j++) {
            Logger::raw(" ");
        }
        Logger::raw(": ");
        Logger::rawln(commands[i].description);
    }
    Logger::rawln("==========================");
}

void TerminalCLI::handle() {
    while (Serial.available() > 0) {
        char c = Serial.read();
        
        if (c == '\n' || c == '\r') {
            if (serialInputBuffer.length() > 0) {
                Serial.println(); // Echo newline
                String input = serialInputBuffer;
                input.trim();
                serialInputBuffer = ""; // Reset buffer
                
                int spaceIndex = input.indexOf(' ');
                String cmdName = (spaceIndex == -1) ? input : input.substring(0, spaceIndex);
                String args = (spaceIndex == -1) ? "" : input.substring(spaceIndex + 1);
                
                bool found = false;
                if (cmdName == "help" || cmdName == "?") {
                    printHelp();
                    found = true;
                } else {
                    for (int i = 0; i < commandCount; i++) {
                        if (cmdName == commands[i].name) {
                            commands[i].handler(args);
                            found = true;
                            break;
                        }
                    }
                }
                
                if (!found) {
                    Logger::warn("Unknown command! Type 'help' for a list of available commands.");
                }
            }
        } else if (c == '\b' || c == 127) { // Handle backspace or delete
            if (serialInputBuffer.length() > 0) {
                serialInputBuffer.remove(serialInputBuffer.length() - 1);
                Serial.print("\b \b"); // Erase character from terminal
            }
        } else {
            // Accumulate characters until Enter is pressed
            serialInputBuffer += c;
            Serial.print(c); // Echo character
        }
    }
}
