#include "Commands.h"
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

static void printUsage()
{
    std::cerr << "Commands:\n"
              << "  pre_process\n"
              << "  run_query <start_range> <end_range> <buffer_size>\n"
              << "  quit\n";
}

int main()
{
    printUsage();
    std::string line;
    while (true) {
        std::cout << "> " << std::flush;
        if (!std::getline(std::cin, line) || line == "quit" || line == "exit") break;
        std::istringstream iss(line);
        std::vector<std::string> tokens;
        std::string tok;
        while (iss >> tok) tokens.push_back(tok);
        if (tokens.empty()) continue;

        if (tokens[0] == "pre_process") {
            if (tokens.size() != 1) {
                std::cerr << "Usage: pre_process\n";
                continue;
            }
            pre_process();
        } else if (tokens[0] == "run_query") {
            if (tokens.size() != 4) {
                std::cerr << "Usage: run_query <start> <end> <buffer_size>\n";
                continue;
            }
            int buf = 0;
            try {
                buf = std::stoi(tokens[3]);
            } catch (...) {
                std::cerr << "Error: buffer_size must be an integer\n";
                continue;
            }
            run_query(tokens[1], tokens[2], buf);
        } else {
            std::cerr << "Unknown command: " << tokens[0] << "\n";
            printUsage();
        }
    }
    return 0;
}
