#include <iostream>
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
  Processor processor = Processor(&state);
  View* view = View::Init(&processor, visual, hidectrl);

  // main execution
  //Util::SaveState(&state, "foosave");
  //Util::LoadState(&state, "fooload");
  view->Exec();

  // finalise
  //delete processor;

  return 0;
}


// Help: Print usage help.

static void
Help()
{
  std::cout << "Usage: " << ME << " [OPTIONS]\n\n"
            << "Primordial particle system visualiser/processor.\n\n"
            << "Options:\n"
            << "  -f FILE  supply an initial state\n"
            << "  -c       run in headless mode\n"
            << "  -u       hide the visualiser controls\n"
            << "  -v       show version\n"
            << "  -h       show this help" << std::endl;
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
      std::cout << ME << " version " << VERSION << std::endl;
      return;
    }
    else if ("bad_file" == opt)
    {
      std::cerr << "Error: no file provided\n\n";
    }
    else if ("inputstate" == opt)
    {
      std::cerr << "Error: unreadable file '" << opts["inputstate"] << "'\n\n";
    }
    else
    {
      std::cerr << "Error: unknown argument '" << opts["quit"] << "'\n\n";
    }
    Help();
    return;
  }
  if (opts["headless"].empty())
  {
    std::cout << "Running visualiser" << std::endl;
  }
  else
  {
    opt = opts["inputstate"];
    std::cout << "Running headless";
    if (! opt.empty()) std::cout << ": " << opt;
    std::cout << std::endl;
  }
}

