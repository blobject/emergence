#define CL_HPP_TARGET_OPENCL_VERSION 210
#define MESA_GL_VERSION_OVERRIDE 3.3
#define MESA_GLSL_VERSION_OVERRIDE 330

#include <fstream>
#include <map>
#include <unistd.h>

#include "util/common.hh"
#include "util/log.hh"
#include "util/util.hh"
#include "proc/proc.hh"
#include "view/view.hh"


static std::map<std::string,std::string>
Args(int argc, char* argv[]);

static void
Argue(Log &log, std::map<std::string,std::string> &opts);


// main: Emergence program entry point.

int
main(int argc, char* argv[])
{
  // logger object
  Log log = Log(128);

  // arguments
  auto opts = Args(argc, argv);
  Argue(log, opts);
  if (! opts["return"].empty()) return std::stoi(opts["return"]);

  // configuration
  std::string init = opts["inputstate"];
  bool visual = opts["headless"].empty();
  bool hide_ctrl = ! opts["hidectrl"].empty();

  // system objects
  State state = State(log, init);
  Proc proc = Proc(log, state);
  std::unique_ptr<View> view =
    std::move(View::Init(log, state, proc, visual, hide_ctrl));

  // execution
  view->Exec();

  return 0;
}


// Help: Print usage help.

static void
Help()
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
  while (-1 != (opt = getopt(argc, argv, ":cf:ghv")))
  {
    switch (opt)
    {
      case 'g': opts["headless"] = "y"; break;
      case 'c': opts["hidectrl"] = "y"; break;
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
Argue(Log &log, std::map<std::string,std::string> &opts)
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
      log.Add(Attn::O, std::string(ME) + " version " + std::string(VERSION));
      return;
    }
    else if ("bad_file" == opt)
    {
      log.Add(Attn::E, "no file provided\n\n");
    }
    else if ("inputstate" == opt)
    {
      log.Add(Attn::E, "unreadable file '" + opts["inputstate"] + "'\n");
    }
    else
    {
      log.Add(Attn::E, "unknown argument '" + opts["quit"] + "'\n");
    }
    Help();
    return;
  }
  if (opts["headless"].empty())
  {
    log.Add(Attn::O, "Running emergence canvas");
  }
  else
  {
    opt = opts["inputstate"];
    std::string out = "Running emergence headless";
    if (! opt.empty()) out += ": " + opt;
    log.Add(Attn::O, out);
  }
}

