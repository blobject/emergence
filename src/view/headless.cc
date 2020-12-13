#include "headless.hh"
#include "../util/util.hh"
#include <cstdlib>
#include <signal.h>


// Global data for sigint handling

static Headless* hoc;


Headless::Headless(Log& log, Control& ctrl)
    : log_(log), ctrl_(ctrl)
{
    hoc = this;
    log.attach(*this);
    ctrl.attach_to_proc(*this);
    struct sigaction sig;
    sig.sa_handler = this->sigint_callback;
    sigemptyset(&sig.sa_mask);
    sig.sa_flags = 0;
    sigaction(SIGINT, &sig, NULL);
    this->tell_pause();
}


Headless::~Headless()
{
    this->log_.detach(*this);
    this->ctrl_.detach_from_proc(*this);
}


// exec: Take account of iterations.

void
Headless::exec()
{
    // flushing makes ticking smooth but slows processing
    if (-1 == this->ctrl_.stop_) {
        std::cout << "   Processing until eternity...\r";
    } else {
        std::cout << "   Tick " << this->ctrl_.stop_ << "          \r"
                  << std::flush;
    }
}


// react: Headless observes, Log::add(), Proc::next(), Proc::done().

void
Headless::react(Issue issue)
{
    if (Issue::ProcNextDone == issue) {
        this->exec();
    } else if (Issue::NewMessage == issue) {
        std::cout << this->log_.messages_.front().second << std::endl;
    } else if (Issue::ProcDone == issue) {
        this->report(When::Done);
        this->ctrl_.quit();
    }
}


// tell_pause: Print message about pausing

inline void
Headless::tell_pause() const
{
    std::cout << "\nPress Ctrl-C to pause.\n" << std::endl;
}


// report: Print out processed information

void
Headless::report(When when) const
{
    Control &ctrl = this->ctrl_;
    State &state = ctrl.get_state();
    if (When::Done == when) {
        std::cout << "\nProcessing finished after "
                  << ctrl.start_ - ctrl.stop_ << " ticks.";
    } else if (When::Paused == when) {
        std::cout << "\nPaused after " << ctrl.start_ - ctrl.stop_ << " ticks.";
    }
    std::cout << "\n  num:    " << state.num_
              << "\n  width:  " << state.width_
              << "\n  height: " << state.height_
              << "\n  alpha:  " << state.alpha_
              << " (" << Util::rad_to_deg(state.alpha_) << " deg)"
              << "\n  beta:   " << state.beta_
              << " (" << Util::rad_to_deg(state.beta_) << " deg)"
              << "\n  scope:  " << state.scope_
              << "\n  speed:  " << state.speed_;
}


// sigint_callback: Handle Ctrl-C

void
Headless::sigint_callback(int signal)
{
    if (SIGINT != signal)
    {
        return;
    }

    Control &ctrl = hoc->ctrl_;
    std::string prompt_menu =
        "\n(R)ESUME, (P)RINT info, (Q)UIT, (S)AVE & quit, (L)OAD new: ";

    std::cout << prompt_menu << std::flush;
    char key;
    while (true) {
        system("stty raw");
        key = getchar();
        system("stty cooked");
        if (std::string("LlPpQqRrSs").find(key) == std::string::npos) {
            std::cout << prompt_menu << std::flush;
            continue;
        }
        if ('P' == key || 'p' == key) {
            hoc->report(When::Paused);
            std::cout << prompt_menu << std::flush;
            continue;
        }
        if ('Q' == key || 'q' == key) {
            hoc->report(When::Done);
            ctrl.quit();
            return;
        }
        if ('R' == key || 'r' == key) {
            std::cout << "\n";
            hoc->tell_pause();
            return;
        }
        if ('L' == key || 'l' == key) {
            std::cout << "\nLoad file: " << std::flush;
            std::string file;
            std::cin >> file;
            if (-1 == ctrl.load(file).num) {
                std::cerr << "\nLoading from \"" << file << "\" failed!"
                          << std::endl;
                continue;
            }
            return;
        }
        if ('S' == key || 's' == key) {
            std::cout << "\nSave file: " << std::flush;
            std::string file;
            std::cin >> file;
            if (!ctrl.save(file)) {
                std::cerr << "\nSaving to \"" << file << "\" failed!"
                          << std::endl;
                continue;
            }
            ctrl.quit();
            return;
        }
    }
}

