#define CL_HPP_TARGET_OPENCL_VERSION 210
#define MESA_GL_VERSION_OVERRIDE 3.3
#define MESA_GLSL_VERSION_OVERRIDE 330

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


// main: Emergence program entry point.

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

    /* dependency & observer graph
     * ----------   ........
     *
     *   .................................
     *   v                               :
     * State <---- Proc <-- Cl           :       .-- Gl
     *         |    ^              .-- Canvas <--|
     *         |    |              |             '-- Gui
     *         '-- Ctrl <-- View <---- Headless
     */
    // system objects
    auto state = State(log);
    auto cl = Cl(log);
    auto proc = Proc(log, state, cl);
    auto ctrl = Control(log, state, proc, init);
    std::unique_ptr<View> view =
        std::move(View::init(log, ctrl, headless, hide_side));

    // execution
    while (!ctrl.quit_) {
        ctrl.next();
    }

    return 0;
}


// help: Print usage help.

static void
help()
{
    std::cout << "Usage: " << std::string(ME) << " [OPTIONS]\n\n"
              << "Primordial particle system visualiser/processor.\n\n"
              << "Options:\n"
              << "  -f FILE  supply an initial state\n"
              << "  -g       run in headless mode\n"
              << "  -c       hide the visualiser controls\n"
              << "  -v       show version\n"
              << "  -h       show this help"
              << std::endl;
}


// args: Parse commandline arguments.

static std::map<std::string,std::string>
args(int argc, char* argv[])
{
    std::map<std::string,std::string> opts = {{"quit", ""},
                                              {"return", ""},
                                              {"headless", ""},
                                              {"inputstate", ""},
                                              {"hidectrl", ""}};
    std::ifstream stream;
    int opt;
    while (-1 != (opt = getopt(argc, argv, ":cf:ghv"))) {
        switch (opt) {
        case 'g': opts["headless"] = "y"; break;
        case 'c': opts["hidectrl"] = "y"; break;
        case 'v': opts["quit"] = "version";  opts["return"] =  "0"; break;
        case 'h': opts["quit"] = "help";     opts["return"] =  "0"; break;
        case ':': opts["quit"] = "bad_file"; opts["return"] = "-1"; break;
        case 'f':
            opts["inputstate"] = optarg;
            stream = std::ifstream(optarg);
            if (stream) {
                stream.close();
            } else {
                opts["quit"] = "inputstate";
                opts["return"] = "-1";
            }
            break;
        case '?':
        default: opts["quit"] = optopt; opts["return"] = "-1"; break;
        }
    }
    return opts;
}


// argue: Print appropriate messages according to commandline arguments.

static void
argue(Log& log, std::map<std::string,std::string>& opts)
{
    auto opt = opts["quit"];
    if (! opt.empty()) {
        if ("help" == opt) {
            help();
            return;
        } else if ("version" == opt) {
            log.add(Attn::O, std::string(ME) + " version "
                    + std::string(VERSION));
            return;
        } else if ("bad_file" == opt) {
            log.add(Attn::E, "no file provided\n\n");
        } else if ("inputstate" == opt) {
            log.add(Attn::E, "unreadable file '" + opts["inputstate"] + "'\n");
        } else {
            log.add(Attn::E, "unknown argument '" + opts["quit"] + "'\n");
        }
        help();
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
    log.add(Attn::O, message);
    std::cout << message << "\n";
}

