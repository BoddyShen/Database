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

int processCommand(const std::vector<std::string>& tokens, bool interactive) {
    if (tokens.empty()) {
        return interactive ? EXIT_SUCCESS : EXIT_FAILURE; // Nothing to do in interactive, error if cmd line expected command
    }

    const std::string& command = tokens[0];

    if (!interactive && (command == "quit" || command == "exit")) {
         std::cerr << "Error: 'quit'/'exit' commands are only valid in interactive mode.\n";
         return EXIT_FAILURE;
    }
     if (interactive && (command == "quit" || command == "exit")) {
         return -1; // Special signal to exit interactive loop
    }


    if (command == "pre_process") {
        if (tokens.size() != 1 && tokens.size() != 2) {
            std::cerr << "Usage: pre_process [test]\n";
            return interactive ? EXIT_SUCCESS : EXIT_FAILURE; // Continue prompt in interactive, fail in cmd line
        }
        bool test = (tokens.size() == 2 && tokens[1] == "test");
        pre_process(test); // Assuming pre_process handles its own errors/output

    } else if (command == "run_query") {
        if (tokens.size() != 4 && tokens.size() != 5) {
            std::cerr << "Usage: run_query <start> <end> <buffer_size> [test]\n";
             return interactive ? EXIT_SUCCESS : EXIT_FAILURE; // Continue prompt in interactive, fail in cmd line
        }
        int buf = 0;
        try {
            buf = std::stoi(tokens[3]);
             if (buf <= 0) { // Basic validation
                 std::cerr << "Error: buffer_size must be a positive integer\n";
                 return interactive ? EXIT_SUCCESS : EXIT_FAILURE;
             }
        } catch (const std::invalid_argument& e) {
            std::cerr << "Error: buffer_size '" << tokens[3] << "' is not a valid integer.\n";
             return interactive ? EXIT_SUCCESS : EXIT_FAILURE;
        } catch (const std::out_of_range& e) {
             std::cerr << "Error: buffer_size '" << tokens[3] << "' is out of range.\n";
             return interactive ? EXIT_SUCCESS : EXIT_FAILURE;
        } catch (...) {
            std::cerr << "Error: invalid buffer_size '" << tokens[3] << "'\n";
             return interactive ? EXIT_SUCCESS : EXIT_FAILURE;
        }

        bool test = (tokens.size() == 5 && tokens[4] == "test");
        run_query(tokens[1], tokens[2], buf, test); // Assuming run_query handles its own errors/output

    } else {
        std::cerr << "Unknown command: " << command << "\n";
        printUsage();
         return interactive ? EXIT_SUCCESS : EXIT_FAILURE; // Continue prompt in interactive, fail in cmd line
    }

    return EXIT_SUCCESS; // Command executed successfully
}



int main(int argc, char* argv[])
{
    if (argc > 1) {
        // Command-line mode: Process arguments directly
        std::vector<std::string> tokens;
        // Start from argv[1] to skip the program name
        for (int i = 1; i < argc; ++i) {
            // Check for null just in case, though argv[argc] should be NULL
            if (argv[i]) {
                tokens.push_back(argv[i]);
            }
        }
         // Call processCommand in non-interactive mode, return its exit code
        return processCommand(tokens, false);

    } else {
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
                if (tokens.size() != 1 && tokens.size() != 2) {
                    std::cerr << "Usage: pre_process\n";
                    continue;
                }
                if (tokens.size() == 2) {
                    // test mode
                    bool test = (tokens[1] == "test");
                    pre_process(test);
                } else {
                    pre_process();
                }
            } else if (tokens[0] == "run_query") {
                if (tokens.size() != 4 && tokens.size() != 5) {
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
                if (tokens.size() == 4) {
                    run_query(tokens[1], tokens[2], buf);
                } else if (tokens.size() == 5) {
                    // test mode
                    bool test = (tokens[4] == "test");
                    run_query(tokens[1], tokens[2], buf, test);
                }
            } else {
                std::cerr << "Unknown command: " << tokens[0] << "\n";
                printUsage();
            }
        }
        return 0;
    }
}
