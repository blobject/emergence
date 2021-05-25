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
  this->current_ = "";
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

  if (ctrl.expctrl_.experiment_) {
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
Headless::intro()
{
  this->tell_usage();
}


void
Headless::react(Issue issue)
{
  if (Issue::ProcNextDone == issue) {
    this->exec();
  } else if (Issue::NewMessage == issue && !this->ctrl_.expctrl_.experiment_) {
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
  State& state = ctrl.state_;
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
Headless::prompt_base()
{
  Control& ctrl = this->ctrl_;
  std::string menu_base = "\n\nbase menu:\n[R]ESUME\n[P]RINT parameters\n[C]ONFIG parameters\n[A]NALYSE\n[S]AVE state\n[L]OAD state\n[Q]UIT\n> ";

  int key;
  while (true) {
    if ('\n' != key) {
      std::cout << menu_base << std::flush;
    }
    system("stty raw");
    key = getchar();
    system("stty cooked");
    if (std::string("AaCcLlPpQqRrSs").find(key) == std::string::npos) {
      continue;
    }

    if ('A' == key || 'a' == key) { this->prompt_analyse(); continue; }
    if ('C' == key || 'c' == key) { this->prompt_config(); continue; }
    if ('P' == key || 'p' == key) { this->report(When::Paused); continue; }
    if ('Q' == key || 'q' == key) {
      this->report(When::Done);
      ctrl.quit();
      std::cout << std::endl;
      return;
    }
    if ('R' == key || 'r' == key) {
      std::cout << "\n";
      this->tell_usage();
      return;
    }
    if ('L' == key || 'l' == key) {
      this->current_ = "\nLoad from file> ";
      std::string file = this->prompt_file(this->current_);
      this->current_ = "";
      if (file.empty()) {
        std::cerr << "Loading canceled." << std::flush;
        continue;
      }
      if (-1 == ctrl.load(file).num) {
        continue;
      }
      this->tell_usage();
      return;
    }
    if ('S' == key || 's' == key) {
      this->current_ = "\nSave to file> ";
      std::string file = this->prompt_file(this->current_);
      this->current_ = "";
      if (file.empty()) {
        std::cerr << "Saving canceled." << std::flush;
        continue;
      }
      if (!ctrl.save(file)) {
        std::cerr << "Saving to \"" << file << "\" failed!" << std::flush;
        continue;
      }
      this->tell_usage();
      return;
    }
  }
}


void
Headless::prompt_config()
{
  UiState& uistate = this->uistate_;
  std::string menu_config = "\n\nconfig:\n[B]ACK to base menu";

  long n;
  float f;
  int key;
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
      this->current_ = "\nnum (int, 1<=x<=1bil)> ";
      n = this->prompt_int(this->current_, 1, 1000000000, true, true);
      this->current_ = "";
      if (0 > n) {
        std::cerr << "Config canceled." << std::flush;
        continue;
      }
      uistate.num_ = static_cast<unsigned int>(n);
      uistate.deceive();
      continue;
    }
    if ('W' == key || 'w' == key) {
      this->current_ = "\nwidth (int, 1<=x<=1bil)> ";
      n = this->prompt_int(this->current_, 1, 1000000000, true, true);
      this->current_ = "";
      if (0 > n) {
        std::cerr << "Config canceled." << std::flush;
        continue;
      }
      uistate.width_ = static_cast<unsigned int>(n);
      uistate.deceive();
      continue;
    }
    if ('H' == key || 'h' == key) {
      this->current_ = "\nheight (int, 1<=x<=1bil)> ";
      n = this->prompt_int(this->current_, 1, 1000000000, true, true);
      this->current_ = "";
      if (0 > n) {
        std::cerr << "Config canceled." << std::flush;
        continue;
      }
      uistate.height_ = static_cast<unsigned int>(n);
      uistate.deceive();
      continue;
    }
    if ('1' == key) {
      this->current_ = "\nalpha (float degrees, 0<=x<360)> ";
      f = this->prompt_float(this->current_, 0.0f, 360.0f, true, false);
      this->current_ = "";
      if (0 > f) {
        std::cerr << "Config canceled." << std::flush;
        continue;
      }
      uistate.alpha_ = f;
      uistate.deceive();
      continue;
    }
    if ('2' == key) {
      this->current_ = "\nbeta (float degrees, 0<=x<360)> ";
      f = this->prompt_float(this->current_, 0.0f, 360.0f, true, false);
      this->current_ = "";
      if (0 > f) {
        std::cerr << "Config canceled." << std::flush;
        continue;
      }
      uistate.beta_ = f;
      uistate.deceive();
      continue;
    }
    if ('V' == key || 'v' == key) {
      this->current_ = "\nscope (float, 1<=x<=1bil)> ";
      f = this->prompt_float(this->current_, 0.0f, 1000000000.0f, true, true);
      this->current_ = "";
      if (0 > f) {
        std::cerr << "Config canceled." << std::flush;
        continue;
      }
      uistate.scope_ = f;
      uistate.deceive();
      continue;
    }
    if ('A' == key || 'a' == key) {
      this->current_ = "\nascope (float, 1<=x<=1bil)> ";
      f = this->prompt_float(this->current_, 0.0f, 1000000000.0f, true, true);
      this->current_ = "";
      if (0 > f) {
        std::cerr << "Config canceled." << std::flush;
        continue;
      }
      uistate.ascope_ = f;
      uistate.deceive();
      continue;
    }
    if ('S' == key || 's' == key) {
      this->current_ = "\nspeed (float, 0<x<=1bil)> ";
      f = this->prompt_float(this->current_, 0.0f, 1000000000.0f, false, true);
      this->current_ = "";
      if (0 > f) {
        std::cerr << "Config canceled." << std::flush;
        continue;
      }
      uistate.speed_ = f;
      uistate.deceive();
      continue;
    }
  }
}


void
Headless::prompt_analyse()
{
  Control& ctrl = this->ctrl_;
  UiState& uistate = this->uistate_;
  std::string menu_analyse = "\n\nanalyse:\n[B]ACK to base menu\n[C]LUSTER particles\nIN[J]ECT a cluster\n[I]NSPECT\n> ";

  int key;
  while (true) {
    std::cout << menu_analyse << std::flush;
    system("stty raw");
    key = getchar();
    system("stty cooked");
    if (std::string("BbCcIiJj").find(key) == std::string::npos) {
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
      this->prompt_inspect();
      continue;
    }
    if ('J' == key || 'j' == key) {
      this->prompt_inject();
      continue;
    }
  }
}


void
Headless::prompt_inject() const
{
  Control& ctrl = this->ctrl_;
  UiState& uistate = this->uistate_;
  std::string menu_analyse = "\n\ninject (capital to include surrounding nutrients):\n[B]ACK to analyse menu\n[Nn]utrients\npremature [Ss]pore\n[Mm]ature spore\n[Rr]ing\npremature [Cc]ell\n[Tt]riangle cell\ns[Qq]uare cell\n[Pp]entagon cell\n> ";

  int key;
  while (true) {
    std::cout << menu_analyse << std::flush;
    system("stty raw");
    key = getchar();
    system("stty cooked");
    if (std::string("BbCcMmNnPpQqRrSsTt").find(key) == std::string::npos) {
      continue;
    }
    if ('B' == key || 'b' == key) {
      return;
    }

    bool cap = 'A' <= key && key <= 'Z';
    Type type = Type::Nutrient;
    if      ('C' == key || 'c' == key) { type = Type::PrematureCell; }
    else if ('M' == key || 'm' == key) { type = Type::MatureSpore; }
    else if ('N' == key || 'n' == key) { type = Type::Nutrient; }
    else if ('P' == key || 'p' == key) { type = Type::PentagonCell; }
    else if ('Q' == key || 'q' == key) { type = Type::SquareCell; }
    else if ('R' == key || 'r' == key) { type = Type::Ring; }
    else if ('S' == key || 's' == key) { type = Type::PrematureSpore; }
    else if ('T' == key || 't' == key) { type = Type::TriangleCell; }
    std::cout << "\n" << ctrl.inject(type, cap) << std::flush;
  }
}


void
Headless::prompt_inspect()
{
  Control& ctrl = this->ctrl_;
  UiState& uistate = this->uistate_;
  Exp& exp = ctrl.exp_;
  std::string menu_inspect = "\n\ninspect:\n[B]ACK to analyse menu\n[P]ARTICLE\n[C]LUSTER\n> ";

  int key;
  while (true) {
    std::cout << menu_inspect << std::flush;
    system("stty raw");
    key = getchar();
    system("stty cooked");
    if (std::string("BbCcPp").find(key) == std::string::npos) {
      continue;
    }
    if ('B' == key || 'b' == key) {
      return;
    }

    if ('C' == key || 'c' == key) {
      ctrl.cluster(uistate.scope_, 14);
      unsigned int num = exp.clusters_.size();
      if (!num) {
        std::cout << "\nNo clusters detected." << std::flush;
        continue;
      }
      std::ostringstream message;
      message << "\nwhich cluster? (0<=x<=" << num - 1 << ")> ";
      this->current_ = message.str();
      long n = this->prompt_int(this->current_, 0, num, true, true);
      this->current_ = "";
      if (0 > n) {
        std::cerr << "Cluster inspection canceled." << std::flush;
        continue;
      }
      std::string type = "unknown";
      if (0 < exp.cell_clusters_.count(n)) {
        type = "cell";
      } else if (0 < exp.spore_clusters_.count(n)) {
        type = "spore";
      }
      message.str("");
      message << "\ncluster: " << n
              << "\ntype: " << type
              << "\n" << exp.clusters_[n].size() << " particles:";
      for (unsigned int p : exp.clusters_[n]) {
        message << " " << p;
      }
      std::cout << message.str() << std::flush;
      continue;
    }
    if ('P' == key || 'p' == key) {
      State& state = ctrl.state_;
      unsigned int num = uistate.num_;
      std::ostringstream message;
      message << "\nwhich particle? (0<=x<=" << num - 1 << ")> ";
      this->current_ = message.str();
      long n = this->prompt_int(this->current_, 0, num, true, true);
      this->current_ = "";
      if (0 > n) {
        std::cerr << "Particle inspection canceled." << std::flush;
        continue;
      }
      message.str("");
      message << std::fixed << std::setprecision(3)
              << "\nparticle: " << n
              << "\ntype: " << state.type_name(state.pt_[n])
              << "\nx: " << state.px_[n]
              << "\ny: " << state.py_[n]
              << "\nphi: " << Util::rad_to_deg(state.pf_[n])
              << "\nn: " << state.pn_[n]
              << "\nl: " << state.pl_[n]
              << "\nr: " << state.pr_[n];
      std::cout << message.str() << std::flush;
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
    << "\n(1)alpha, " << current.alpha
    << "\n(2)beta,  " << current.beta
    << "\n(v)scope, " << current.scope
    << "\n[a]scope, " << current.ascope
    << "\n[s]peed,  " << current.speed;
  return s.str();
}


std::string
Headless::prompt_file(const std::string& ask) const
{
  std::string input;
  int key;
  while (true) {
    std::cout << ask << std::flush;
    system("stty eof ''");
    std::getline(std::cin, input);
    system("stty cooked");
    return Util::trim(input);
  }
}


long
Headless::prompt_int(
  const std::string& ask,
  unsigned int low, unsigned int high, bool low_inc, bool high_inc
) const {
  std::string input;
  long n;
  while (true) {
    std::cout << ask << std::flush;
    system("stty eof ''");
    std::getline(std::cin, input);
    system("stty cooked");
    if (input.empty()) {
      return -1;
    }
    try {
      n = std::stol(input);
    } catch (std::invalid_argument /* error */) {
      std::cerr << "Not a valid integer." << std::endl;
      continue;
    }
    if ((low_inc   && low  >  n) ||
        (!low_inc  && low  >= n) ||
        (high_inc  && high <  n) ||
        (!high_inc && high <= n))
    {
      std::cerr << "Not within range." << std::endl;
      continue;
    }
    return n;
  }
}


float
Headless::prompt_float(
  const std::string& ask, float low, float high, bool low_inc, bool high_inc
) const {
  std::string input;
  float f;
  while (true) {
    std::cout << ask << std::flush;
    system("stty eof ''");
    std::getline(std::cin, input);
    system("stty cooked");
    if (input.empty()) {
      return -1.0f;
    }
    try {
      f = std::stof(input);
    } catch (std::invalid_argument /* error */) {
      std::cerr << "Not a valid float." << std::endl;
      continue;
    }
    if ((low_inc   && low  >  f) ||
        (!low_inc  && low  >= f) ||
        (high_inc  && high <  f) ||
        (!high_inc && high <= f))
    {
      std::cerr << "Not within range." << std::endl;
      continue;
    }
    return f;
  }
}


void
Headless::sigint_callback(int /* signal */)
{
  Control& ctrl = hoc->ctrl_;

  if (ctrl.expctrl_.experiment_) {
    ctrl.quit();
    return;
  }

  if (!hoc->current_.empty()) {
    std::cout << hoc->current_ << std::flush;
    return;
  }

  hoc->prompt_base();
}

