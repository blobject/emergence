#include <fstream>
#include <map>
#include <unistd.h>

#include "util/common.hh"
#include "util/util.hh"
#include "processor/processor.hh"
#include "view/view.hh"


// forward declarations

static std::map<std::string,std::string>
  Args(int argc, char* argv[]);

static void
  Argue(std::map<std::string,std::string> &opts);


// main: Emergence program entry point.

int
main(int argc, char* argv[])
{
  // arguments
  auto opts = Args(argc, argv);
  Argue(opts);
  if (! opts["return"].empty()) return std::stoi(opts["return"]);

  // configuration
  std::string load = opts["inputstate"];
  bool visual = opts["headless"].empty();
  bool hidectrl = opts["hidectrl"].empty();

  // main objects
  State state = State(load);
  Processor processor = Processor(state);
  std::unique_ptr<View> view = std::move(
    View::Init(&processor, visual, hidectrl));

  // main execution
  //Util::SaveState(&state, "foosave");
  //Util::LoadState(&state, "fooload");
  view->Exec();

  return 0;
}


// Help: Print usage help.

static void
Help()
{
  Util::Out("Usage: " + std::string(ME) + " [OPTIONS]\n\n"
            + "Primordial particle system visualiser/processor.\n\n"
            + "Options:\n"
            + "  -f FILE  supply an initial state\n"
            + "  -c       run in headless mode\n"
            + "  -u       hide the visualiser controls\n"
            + "  -v       show version\n"
            + "  -h       show this help");
}


// Args: Parse commandline arguments.

static std::map<std::string,std::string>
Args(int argc, char* argv[])
{
  std::map<std::string,std::string> opts = {{"quit", ""},
                                            {"return", ""},
                                            {"headless", ""},
                                            {"inputstate", ""},
                                            {"hidectrl", ""}};
  std::ifstream stream;
  int opt;
  while (-1 != (opt = getopt(argc, argv, ":cf:huv")))
  {
    switch (opt)
    {
      case 'c': opts["headless"]  = "y"; break;
      case 'u': opts["hidectrl"] = "y"; break;
      case 'v': opts["quit"] = "version";  opts["return"] =  "0"; break;
      case 'h': opts["quit"] = "help";     opts["return"] =  "0"; break;
      case ':': opts["quit"] = "bad_file"; opts["return"] = "-1"; break;
      case 'f':
        opts["inputstate"] = optarg;
        stream = std::ifstream(optarg);
        if (stream)
        {
          stream.close();
        }
        else
        {
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


// Argue: Print appropriate messages according to commandline arguments.

static void
Argue(std::map<std::string,std::string> &opts)
{
  auto opt = opts["quit"];
  if (! opt.empty())
  {
    if ("help" == opt)
    {
      Help();
      return;
    }
    else if ("version" == opt)
    {
      Util::Out(std::string(ME) + " version " + std::string(VERSION));
      return;
    }
    else if ("bad_file" == opt)
    {
      Util::Err("no file provided\n\n");
    }
    else if ("inputstate" == opt)
    {
      Util::Err("unreadable file '" + opts["inputstate"] + "'\n");
    }
    else
    {
      Util::Err("unknown argument '" + opts["quit"] + "'\n");
    }
    Help();
    return;
  }
  if (opts["headless"].empty())
  {
    Util::Out("Running emergence visualiser");
  }
  else
  {
    opt = opts["inputstate"];
    std::string out = "Running emergence headless";
    if (! opt.empty()) out += ": " + opt;
    Util::Out(out);
  }
}

