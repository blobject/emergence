//===-- main.cc - program entry point --------------------------*- C++ -*-===//

#include "util/common.hh"
#include "util/log.hh"
#include "view/view.hh"
#include <fstream>
#include <map>
#include <unistd.h> // getopt, optarg, optopt


static std::map<std::string,std::string>
args(int argc, char* argv[]);

static void
argue(Log& log, std::map<std::string,std::string>& opts);


/// main(): Emergence program entry point containing the processing loop.
int
main(int argc, char* argv[])
{
    // logger object
    Log log = Log(256);

    // arguments
    std::map<std::string,std::string> opts = args(argc, argv);
    argue(log, opts);
    if (!opts["return"].empty()) {
        return std::stoi(opts["return"]);
    }

    // configuration
    std::string init = opts["inputstate"];
    bool headless = !opts["headless"].empty();
    bool hide_side = !opts["hideside"].empty();

    /* dependency & observation graph
     * ----------   ...........
     *
     *   .................................
     *   v          v                    :
     * State <---- Proc <-- Cl           :  .-------> Gl
     *         |    ^              .-- Canvas <--.
     *         |    |              |             '--> Gui
     *         '-- Ctrl <-- View <---- Headless
     */
    // system objects
    auto state = State(log);
    auto cl = Cl(log); // stub object if OpenCL is unavailable
    auto proc = Proc(log, state, cl);
    auto ctrl = Control(log, state, proc, init);
    std::unique_ptr<View> view = View::init(log, ctrl, headless, hide_side);

    // execution
    while (!ctrl.quit_) {
        ctrl.next();
    }

    return 0;
}


/// usage(): Print usage oneliner.
static void
usage()
{
    std::cout << "Usage: " << std::string(ME) << " -(f FILE|g|c|v|h?)"
              << std::endl;
}


/// help(): Print usage help.
static void
help()
{
    usage();
    std::cout << "\nPrimordial particle system visualiser/processor.\n\n"
              << "Options:\n"
              << "  -f FILE  supply an initial state\n"
              << "  -g       run in headless mode\n"
              << "  -c       hide the visualiser control side bar\n"
              << "  -v       show version\n"
              << "  -h|-?    show this help"
              << std::endl;
}


/// args(): Parse the arguments to Emergence.
/// \param argc  number of arguments to Emergence
/// \param argv  array of arguments to Emergence
/// \returns  map of program options
static std::map<std::string,std::string>
args(int argc, char* argv[])
{
    std::map<std::string,std::string> opts = {{"quit", ""},
                                              {"return", ""},
                                              {"headless", ""},
                                              {"inputstate", ""},
                                              {"hidectrl", ""}};
    int opt;
    while (-1 != (opt = getopt(argc, argv, "?cf:ghv"))) {
        switch (opt) {
        case 'g': opts["headless"] = "y"; break;
        case 'c': opts["hidectrl"] = "y"; break;
        case 'v': opts["quit"] = "version"; opts["return"] = "0"; break;
        case 'h': case '?': opts["quit"] = "help"; opts["return"] = "0"; break;
        case ':': opts["quit"] = "nofile"; opts["return"] = "-1"; break;
        case 'f': opts["inputstate"] = optarg; break;
        default: opts["quit"] = optopt; opts["return"] = "-1"; break;
        }
    }
    return opts;
}


/// argue(): Print messages according to the arguments to Emergence.
/// \param log  Log object
/// \param opts  map of program options
static void
argue(Log& log, std::map<std::string,std::string>& opts)
{
    std::string opt = opts["quit"];
    if (!opts["inputstate"].empty())
    {
        std::ifstream stream;
        stream = std::ifstream(opts["inputstate"]);
        if (stream) {
            stream.close();
        } else {
            log.add(Attn::E, "unreadable file: " + opts["inputstate"], true);
            opts["return"] = "-1";
            usage();
        }
        return;
    }
    if (!opt.empty()) {
        if ("help" == opt) {
            help();
            return;
        } else if ("version" == opt) {
            log.add(Attn::O, std::string(ME) + " version "
                    + std::string(VERSION), true);
            return;
        } else if ("nofile" == opt) {
            log.add(Attn::E, "no file provided\n", true);
        } else if ("inputstate" == opt) {
        } else {
            log.add(Attn::E, "unknown argument: " + opts["quit"], true);
        }
        usage();
        return;
    }
    std::string message = "Running emergence";
    if (opts["headless"].empty()) {
        message += " canvas";
    }
    else {
        opt = opts["inputstate"];
        message += " headless";
        if (!opt.empty()) message += ": " + opt;
    }
    log.add(Attn::O, message, true);
}

