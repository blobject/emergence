//===-- main.cc - program entry point --------------------------*- C++ -*-===//

#include "util/common.hh"
#include "util/log.hh"
#include "exp/exp.hh"
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
  // arguments prep
  std::map<std::string,std::string> opts = args(argc, argv);

  // logger object (after arguments prep to account for "quiet")
  Log log = Log(256, !opts["quiet"].empty());

  // arguments more
  argue(log, opts);
  if (!opts["return"].empty()) {
    return std::stoi(opts["return"]);
  }

  // configuration
  std::string init = opts["inputstate"];
  bool headless = !opts["headless"].empty();
  bool no_cl = !opts["nocl"].empty();
  bool gui_on = opts["nogui"].empty();
  bool three = opts["two"].empty();

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
  auto proc = Proc(log, state, cl, no_cl);
  auto exp = Exp(log, state);
  auto ctrl = Control(log, state, proc, exp, init);
  auto uistate = UiState(ctrl);
  std::unique_ptr<View> view = View::init(log, ctrl, uistate,
                                          headless, gui_on, three);

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
  std::cout << "Usage: " << std::string(ME) << " -(?h|2|c|f FILE|g|p|q|v)"
            << std::endl;
}


/// help(): Print usage help.
static void
help()
{
  usage();
  std::cout << "\nPrimordial particle system visualiser/processor.\n\n"
            << "Options:\n"
            << "  -?|-h    show this help\n"
            << "  -f FILE  supply an initial state\n"
            << "  -g       run in headless mode\n"
            << "  -p       disable OpenCL\n"
            << "  -v       show version\n\n"
            << "not applicable to headless mode:\n"
            << "  -2       run in 2d mode\n"
            << "  -c       disable GUI and only show canvas\n"
            << "  -q       suppress log to stdout\n"
            << std::endl;
}


/// args(): Parse the arguments to Emergence.
/// \param argc  number of arguments to Emergence
/// \param argv  array of arguments to Emergence
/// \returns  map of program options
static std::map<std::string,std::string>
args(int argc, char* argv[])
{
  std::map<std::string,std::string> opts = {
    {"quit", ""},
    {"headless", ""},
    {"inputstate", ""},
    {"nocl", ""},
    {"nogui", ""},
    {"two", ""},
    {"quiet", ""},
    {"return", ""}
  };
  int opt;
  while (-1 != (opt = getopt(argc, argv, "?2cf:ghpqv"))) {
    if ('?' == opt || 'h' == opt) {
      opts["quit"] = "help";
      opts["return"] = "0";
    }
    else if ('2' == opt) { opts["two"]      = "."; }
    else if ('c' == opt) { opts["nogui"]    = "."; }
    else if ('g' == opt) { opts["headless"] = "."; }
    else if ('p' == opt) { opts["nocl"]     = "."; }
    else if ('q' == opt) { opts["quiet"]    = "."; }
    else if ('v' == opt) { opts["quit"] = "version"; opts["return"] = "0"; }
    else if (':' == opt) { opts["quit"] = "nofile";  opts["return"] = "-1"; }
    else if ('f' == opt) { opts["inputstate"] = optarg; }
    else { opts["quit"] = optopt; opts["return"] = "-1"; }
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
  if (!opts["inputstate"].empty()) {
    std::ifstream stream;
    stream = std::ifstream(opts["inputstate"]);
    if (stream) {
      stream.close();
    } else {
      opts["return"] = "-1";
      log.add(Attn::E, "unreadable file: " + opts["inputstate"], true);
      usage();
    }
    return;
  }
  if (!opt.empty()) {
    if ("help" == opt) {
      help();
      return;
    } else if ("version" == opt) {
      log.add(Attn::O, std::string(ME) + " version " + std::string(VERSION),
              true);
      return;
    } else if ("nofile" == opt) {
      log.add(Attn::E, "no file provided\n", true);
    } else if ("inputstate" == opt) {
      log.add(Attn::E, "trouble with input state\n", true);
    } else {
      log.add(Attn::E, "unknown argument: " + opts["quit"], true);
    }
    usage();
    return;
  }
  std::string message = "Running emergence:";
  if (opts["headless"].empty()) {
    message += " canvas";
    if (opts["nogui"].empty()) {
      message += " & gui";
    } else {
      message += " only";
    }
  } else {
    opt = opts["inputstate"];
    message += " headless";
    if (!opt.empty()) message += ": " + opt;
  }
  log.add(Attn::O, message, true);
}

