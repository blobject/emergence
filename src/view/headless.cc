#include "headless.hh"
#include "../util/util.hh"
#include <signal.h>


// global instance for sigint handling
static Headless* hoc;


Headless::Headless(Log& log, Control& ctrl, UiState& uistate)
  : log_(log), ctrl_(ctrl), uistate_(uistate)
{
  hoc = this;
  log.attach(*this);
  ctrl.attach_to_proc(*this);
  struct sigaction sig;
  sig.sa_handler = this->sigint_callback;
  sigemptyset(&sig.sa_mask);
  sig.sa_flags = 0;
  sigaction(SIGINT, &sig, NULL);
  if (ctrl.paused_) {
    ctrl.paused_ = false;
    this->prompt_base();
    return;
  }
  this->tell_usage();
}


Headless::~Headless()
{
  this->log_.detach(*this);
  this->ctrl_.detach_from_proc(*this);
}


void
Headless::exec()
{
  return; // when profiling
  Control& ctrl = this->ctrl_;

  if (ctrl.experiment_) {
    return;
  }

  if (-1 == ctrl.countdown_) {
    std::cout << "   Processing for eternity... (tick "
              << ctrl.tick_ << ")   \r"
              << std::flush;
  } else {
    // flushing makes ticking appear smooth but slows processing
    std::cout << "   Tick " << ctrl.countdown_ << "          \r"
              << std::flush;
  }
}


void
Headless::react(Issue issue)
{
  if (Issue::ProcNextDone == issue) {
    this->exec();
  } else if (Issue::NewMessage == issue && !this->ctrl_.experiment_) {
    std::cout << this->log_.messages_.front().second << std::endl;
  } else if (Issue::ProcDone == issue) {
    this->report(When::Done);
    this->ctrl_.quit();
  }
}


inline void
Headless::tell_usage() const
{
  std::cout << "\nPress Ctrl-C to interact.\n" << std::endl;
}


void
Headless::report(When when) const
{
  Control& ctrl = this->ctrl_;
  State& state = ctrl.get_state();
  if (When::Done == when) {
    std::cout << "\nProcessing finished after " << ctrl.tick_ << " ticks.";
  } else if (When::Paused == when) {
    std::cout << "\nPaused after " << ctrl.tick_ << " ticks.";
  }
  std::cout << "\n  num:    " << state.num_
            << "\n  width:  " << state.width_
            << "\n  height: " << state.height_
            << "\n  alpha:  " << state.alpha_
            << " (" << Util::rad_to_deg(state.alpha_) << " deg)"
            << "\n  beta:   " << state.beta_
            << " (" << Util::rad_to_deg(state.beta_) << " deg)"
            << "\n  scope:  " << state.scope_
            << "\n  ascope: " << state.ascope_
            << "\n  speed:  " << state.speed_
            << std::flush;
}


void
Headless::prompt_base() const
{
  Control& ctrl = this->ctrl_;
  std::string menu_base = "\n\nbase menu:\n[R]ESUME\n[P]RINT params\n[C]ONFIG params\n[A]NALYSE\n[I]NSPECT\n[S]AVE state & quit\n[L]OAD state\n[Q]UIT\n> ";

  char key;
  while (true) {
    std::cout << menu_base << std::flush;
    system("stty raw");
    key = getchar();
    system("stty cooked");
    if (std::string("AaCcIiLlPpQqRrSs").find(key) == std::string::npos) {
      continue;
    }
    if ('A' == key || 'a' == key) {
      this->prompt_analyse();
      continue;
    }
    if ('C' == key || 'c' == key) {
      this->prompt_config();
      continue;
    }
    if ('I' == key || 'i' == key) {
      this->prompt_inspect();
      continue;
    }
    if ('P' == key || 'p' == key) {
      hoc->report(When::Paused);
      continue;
    }
    if ('Q' == key || 'q' == key) {
      hoc->report(When::Done);
      ctrl.quit();
      std::cout << std::endl;
      return;
    }
    if ('R' == key || 'r' == key) {
      std::cout << "\n";
      hoc->tell_usage();
      return;
    }
    if ('L' == key || 'l' == key) {
      std::cout << "\nLoad file: " << std::flush;
      std::string file;
      std::cin >> file;
      if (-1 == ctrl.load(file).num) {
        std::cerr << "\nLoading from \"" << file << "\" failed!" << std::endl;
        continue;
      }
      return;
    }
    if ('S' == key || 's' == key) {
      std::cout << "\nSave file: " << std::flush;
      std::string file;
      std::cin >> file;
      if (!ctrl.save(file)) {
        std::cerr << "\nSaving to \"" << file << "\" failed!" << std::endl;
        continue;
      }
      ctrl.quit();
      return;
    }
  }
}


void
Headless::prompt_config() const
{
  UiState& uistate = this->uistate_;
  std::string menu_config = "\n\nconfig:\n[B]ACK to base menu";

  unsigned int n;
  float f;
  char key;
  while (true) {
    std::cout << menu_config << tell_config() << "\n> " << std::flush;
    system("stty raw");
    key = getchar();
    system("stty cooked");
    if (std::string("12BbHhNnSsVvWw").find(key) == std::string::npos) {
      continue;
    }
    if ('B' == key || 'b' == key) {
      return;
    }
    if ('N' == key || 'n' == key) {
      n = this->prompt_param_int("\nnum (int, 1<=x<=1bil)> ",
                                 1, 1000000000, true, true);
      uistate.num_ = n;
      uistate.deceive();
      continue;
    }
    if ('W' == key || 'w' == key) {
      n = this->prompt_param_int("\nwidth (int, 1<=x<=1bil)> ",
                                 1, 1000000000, true, true);
      uistate.width_ = n;
      uistate.deceive();
      continue;
    }
    if ('H' == key || 'h' == key) {
      n = this->prompt_param_int("\nheight (int, 1<=x<=1bil)> ",
                                 1, 1000000000, true, true);
      uistate.height_ = n;
      uistate.deceive();
      continue;
    }
    if ('1' == key) {
      f = this->prompt_param_float("alpha (float degrees, 0<=x<360)> ",
                                   0.0f, 360.0f, true, false);
      uistate.alpha_ = f;
      uistate.deceive();
      continue;
    }
    if ('2' == key) {
      f = this->prompt_param_float("\nbeta (float degrees, 0<=x<360)> ",
                                   0.0f, 360.0f, true, false);
      uistate.beta_ = f;
      uistate.deceive();
      continue;
    }
    if ('V' == key || 'v' == key) {
      f = this->prompt_param_float("\nscope (float, 1<=x<=1bil)> ",
                                   0.0f, 1000000000.0f, true, true);
      uistate.scope_ = f;
      uistate.deceive();
      continue;
    }
    if ('A' == key || 'a' == key) {
      f = this->prompt_param_float("\nascope (float, 1<=x<=1bil)> ",
                                   0.0f, 1000000000.0f, true, true);
      uistate.ascope_ = f;
      uistate.deceive();
      continue;
    }
    if ('S' == key || 's' == key) {
      f = this->prompt_param_float("\nspeed (float, 0<x<=1bil)> ",
                                   0.0f, 1000000000.0f, false, true);
      uistate.speed_ = f;
      uistate.deceive();
      continue;
    }
  }
}


void
Headless::prompt_analyse() const
{
  Control& ctrl = this->ctrl_;
  UiState& uistate = this->uistate_;
  std::string menu_analyse = "\n\nanalyse:\n[B]ACK to base menu\n[C]LUSTER particles\n[I]NJECT a cluster\n> ";

  char key;
  while (true) {
    std::cout << menu_analyse << std::flush;
    system("stty raw");
    key = getchar();
    system("stty cooked");
    if (std::string("BbCcDdIi").find(key) == std::string::npos) {
      continue;
    }
    if ('B' == key || 'b' == key) {
      return;
    }
    if ('C' == key || 'c' == key) {
      std::cout << "\n" << ctrl.cluster(uistate.scope_, 14) << std::flush;
      continue;
    }
    if ('I' == key || 'i' == key) {
      std::cout << "\n" << ctrl.inject(Type::MatureSpore, false) << std::flush;
      continue;
    }
  }
}


void
Headless::prompt_inspect() const
{
  Control& ctrl = this->ctrl_;
  UiState& uistate = this->uistate_;
  Exp& exp = ctrl.get_exp();
  std::string menu_inspect = "\n\ninspect:\n[B]ACK to base menu\n[P]ARTICLE\nCLUSTER[S]\n[C]LUSTER\n> ";

  char key;
  while (true) {
    std::cout << menu_inspect << std::flush;
    system("stty raw");
    key = getchar();
    system("stty cooked");
    if (std::string("BbCcPpSs").find(key) == std::string::npos) {
      continue;
    }
    if ('B' == key || 'b' == key) {
      return;
    }
    if ('C' == key || 'c' == key) {
      ctrl.cluster(uistate.scope_, 14);
      std::cout << "\n" << exp.clusters_.size() << std::flush;
      continue;
    }
    if ('P' == key || 'p' == key) {
      std::cout << "\n" << ctrl.get_state().px_[0] << std::flush;
      continue;
    }
    if ('S' == key || 's' == key) {
      ctrl.cluster(uistate.scope_, 14);
      std::cout << "\n" << exp.clusters_.size() << std::flush;
      continue;
    }
  }
}


std::string
Headless::tell_config() const
{
  Stative current = this->uistate_.current();
  std::ostringstream s;
  s << "\n[n]um,    " << current.num
    << "\n[w]idth,  " << current.width
    << "\n[h]eight, " << current.height
    << "\n[1]alpha, " << current.alpha
    << "\n[2]beta,  " << current.beta
    << "\n(v)scope, " << current.scope
    << "\n[a]scope, " << current.ascope
    << "\n[s]peed,  " << current.speed;
  return s.str();
}


unsigned int
Headless::prompt_param_int(
  const std::string& ask_param,
  unsigned int low, unsigned int high, bool low_inc, bool high_inc
) const {
  std::string input;
  long n;
  while (true) {
    std::cout << ask_param << std::flush;
    std::getline(std::cin, input);
    n = std::stol(input);
    if (low_inc   && low  >  n) { continue; }
    if (!low_inc  && low  >= n) { continue; }
    if (high_inc  && high <  n) { continue; }
    if (!high_inc && high <= n) { continue; }
    return static_cast<unsigned int>(n);
  }
}


float
Headless::prompt_param_float(
  const std::string& ask_param,
  float low, float high, bool low_inc, bool high_inc
) const {
  std::string input;
  float f;
  while (true) {
    std::cout << ask_param << std::flush;
    std::getline(std::cin, input);
    f = std::stof(input);
    if (low_inc   && low  >  f) { continue; }
    if (!low_inc  && low  >= f) { continue; }
    if (high_inc  && high <  f) { continue; }
    if (!high_inc && high <= f) { continue; }
    return f;
  }
}


void
Headless::sigint_callback(int /* signal */)
{
  Control& ctrl = hoc->ctrl_;

  if (ctrl.experiment_) {
    ctrl.quit();
    return;
  }

  hoc->prompt_base();
}

