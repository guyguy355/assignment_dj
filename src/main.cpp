#include "DJSession.h"
#include "SetJourney.h"
#include <iostream>
#include <stdexcept>

int main(int argc, char* argv[]) {
    try {
        std::string command = argc > 1 ? argv[1] : "--help";
        if (command == "--help") {
            std::cout << "DJLab - C++ DJ session simulator (no audio playback)\n"
                      << "  ./bin/dj_manager journey <config> <playlist> <report.html>\n"
                      << "  ./bin/dj_manager interactive\n"
                      << "  ./bin/dj_manager demo\n";
            return 0;
        }
        if (command == "journey" && argc == 5) {
            SessionConfig config;
            if (!SessionFileParser::parse_config_file(argv[2], config)) return 1;
            SetJourney::write_report(config, argv[3], argv[4]);
            std::cout << "Report saved: " << argv[4] << std::endl;
            return 0;
        }
        if ((command == "demo" || command == "interactive") && argc == 2) {
            DJSession session("DJLab", command == "demo");
            session.simulate_dj_performance();
            return 0;
        }
        std::cerr << "Invalid arguments. Run --help for usage.\n";
        return 2;
    } catch (const std::exception& e) {
        std::cerr << "DJLab: " << e.what() << std::endl;
        return 1;
    }
}
