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

static void
attempt(Log& log, int exp);


/// main(): Program entry point containing the processing loop.
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
  int experiment = 0;
  if (!opts["exp"].empty()) {
    experiment = std::stoi(opts["exp"]);
  }
  bool headless = !opts["headless"].empty();
  std::string init = opts["input"];
  bool no_cl = !opts["nocl"].empty();
  bool gui_on = opts["nogui"].empty();
  bool pause = !opts["pause"].empty();
  bool three = !opts["three"].empty();

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
  auto state = State(log, experiment);
  auto cl = Cl(log); // stub object if OpenCL is unavailable
  auto proc = Proc(log, state, cl, no_cl);
  auto exp = Exp(log, state, proc, no_cl);
  auto ctrl = Control(log, state, proc, exp, init, pause);
  auto uistate = UiState(ctrl);
  std::unique_ptr<View> view = View::init(log, ctrl, uistate,
                                          headless, gui_on, three);
  log.add(Attn::O, "PID: " + std::to_string(ctrl.pid_), true);

  // execution
  attempt(log, experiment);
  while (!ctrl.quit_) {
    ctrl.next();
  }

  return 0;
}


/// usage(): Print usage oneliner.
static void
usage()
{
  char* me = strdup(ME);
  me[0] += 0x20;
  std::cout << "Usage: " << me
            << " -(?h|3|c|e NUM|g|i FILE|p|q|v|x)"
            << std::endl;
  free(me);
}


/// help(): Print usage help.
static void
help()
{
  usage();
  std::cout << "\nPrimordial particle system visualiser/processor.\n\n"
            << "Options:\n"
            << "  -?|-h    show this help\n"
            << "  -v       show version\n"
            << "  -c       disable OpenCL\n"
            << "  -e NUM   do an experiment\n"
            << "             occupancy:    [11, 12], [13, 14], [15]\n"
            << "             population:   [2]\n"
            << "             heat map:     [31, 32, 33, 34, 35, 36, 37, 38]\n"
            << "             survival:     [41, 42, 43, 44]\n"
            << "             size & noise: [51, 52, 53], [54, 55, 56]\n"
            << "             param sweep:  [6]\n"
            << "  -i FILE  supply an initial state\n"
            << "  -p       start paused\n"
            << "  -x       run in headless mode\n\n"
            << "Options for graphical mode:\n"
            << "  -3       start in 3d mode\n"
            << "  -g       disable GUI and only show canvas\n"
            << "             C-c, C-q: quit\n"
            << "             Space:    pause/resume\n"
            << "             S:        step\n"
            << "  -q       suppress logging to stdout\n"
            << std::endl;
}


/// args(): Parse the arguments to program.
/// \param argc  number of arguments to program
/// \param argv  array of arguments to program
/// \returns  map of program options
static std::map<std::string,std::string>
args(int argc, char* argv[])
{
  std::map<std::string,std::string> opts = {
    {"exp", ""},
    {"headless", ""},
    {"input", ""},
    {"nocl", ""},
    {"nogui", ""},
    {"pause", ""},
    {"quiet", ""},
    {"quit", ""},
    {"return", ""},
    {"three", ""}
  };
  int opt;
  while (-1 != (opt = getopt(argc, argv, "?3ce:gi:hpqvx"))) {
    if ('?' == opt || 'h' == opt) {
      opts["quit"] = "help";
      opts["return"] = "0";
    }
    else if ('3' == opt) { opts["three"] = "."; }
    else if ('c' == opt) { opts["nocl"]  = "."; }
    else if ('e' == opt) { opts["exp"]   = optarg; }
    else if ('g' == opt) { opts["nogui"] = "."; }
    else if ('i' == opt) { opts["input"] = optarg; }
    else if ('p' == opt) { opts["pause"] = "."; }
    else if ('q' == opt) { opts["quiet"] = "."; }
    else if ('v' == opt) { opts["quit"] = "version"; opts["return"] = "0"; }
    else if ('x' == opt) { opts["headless"] = "."; }
    else if (':' == opt) { opts["quit"] = "noarg"; opts["return"] = "-1"; }
    else { opts["quit"] = optopt; opts["return"] = "-1"; }
  }
  return opts;
}


/// argue(): Print messages according to the arguments to program.
/// \param log  Log object
/// \param opts  map of program options
static void
argue(Log& log, std::map<std::string,std::string>& opts)
{
  std::string opt = opts["quit"];
  if (!opts["exp"].empty()) {
    int exp = std::stoi(opts["exp"]);
    auto exps = std::vector<int>{0,
                                 11, 12, 13, 14, 15,
                                 2,
                                 31, 32, 33, 34, 35, 36, 37, 38,
                                 41, 42, 43, 44,
                                 51, 52, 53, 54, 55, 56,
                                 6};
    if (std::find(exps.begin(), exps.end(), exp) != exps.end()) {
      return;
    }
    opts["return"] = "-1";
    log.add(Attn::E, "unknown experiment: " + opts["exp"], true);
    return;
  }
  if (!opts["input"].empty()) {
    std::ifstream stream;
    stream = std::ifstream(opts["input"]);
    if (stream) {
      stream.close();
    } else {
      opts["return"] = "-1";
      log.add(Attn::E, "unreadable file: " + opts["input"], true);
      usage();
    }
    return;
  }
  if (!opt.empty()) {
    if ("help" == opt) {
      help();
      return;
    } else if ("input" == opt) {
      log.add(Attn::E, "trouble with input state\n", true);
    } else if ("noarg" == opt) {
      log.add(Attn::E, "no argument provided\n", true);
    } else if ("version" == opt) {
      log.add(Attn::O, std::string(ME) + " version " + std::string(VERSION),
              true);
    } else {
      log.add(Attn::E, "unknown argument: " + opts["quit"], true);
    }
    usage();
    return;
  }
  std::string message = "Running " + std::string(ME) + ":";
  if (opts["headless"].empty()) {
    message += " canvas";
    if (opts["nogui"].empty()) {
      message += " & gui";
    } else {
      message += " only";
    }
  } else {
    opt = opts["input"];
    message += " headless";
    if (!opt.empty()) message += ": " + opt;
  }
  log.add(Attn::O, message, true);
}


/// attempt(): Print message about specified experiment.
/// \param log  Log object
/// \param exp  specific experiment
static void
attempt(Log& log, int exp)
{
  int expg = 10 <= exp ? exp / 10 : exp;
  std::string message = "Running experiment ";
  if      (1 == expg) { message += "1 \"occupancy\": ";
    if      (11 == exp) { message += "12 particles."; }
    else if (12 == exp) { message += "14 particles."; }
    else if (13 == exp) { message += "0.04 dpe."; }
    else if (14 == exp) { message += "0.07 dpe."; }
    else if (15 == exp) { message += "0.09 dpe."; } }
  else if (2 == expg) { message += "2 \"population\"."; }
  else if (3 == expg) { message += "3 \"heat map\": ";
    if      (31 == exp) { message += "nutrients."; }
    else if (32 == exp) { message += "premature spore."; }
    else if (33 == exp) { message += "mature spore."; }
    else if (34 == exp) { message += "ring."; }
    else if (35 == exp) { message += "premature cell."; }
    else if (36 == exp) { message += "triangle cell."; }
    else if (37 == exp) { message += "square cell."; }
    else if (38 == exp) { message += "pentagon cell."; } }
  else if (4 == expg) { message += "4 \"survival\": ";
    if      (41 == exp) { message += "mature spore survival."; }
    else if (42 == exp) { message += "triangle cell survival."; }
    else if (43 == exp) { message += "mature spore reproduction."; }
    else if (44 == exp) { message += "triangle cell reproduction."; } }
  else if (5 == expg) { message += "5 \"";
    if      (51 == exp) { message += "size\": 0.03 dpe."; }
    else if (52 == exp) { message += "size\": 0.035 dpe."; }
    else if (53 == exp) { message += "size\": 0.04 dpe."; }
    else if (54 == exp) { message += "noise\": 0.03 dpe + noise."; }
    else if (55 == exp) { message += "noise\": 0.035 dpe + noise."; }
    else if (56 == exp) { message += "noise\": 0.04 dpe + noise."; } }
  else if (6 == expg) { message += "6 \"parameter sweep\"."; }
  else if (exp) {
    log.add(Attn::E, "unknown experiment: " + std::to_string(exp), true);
    return;
  }
  if (exp) {
    log.add(Attn::O, message, true);
  }
}

