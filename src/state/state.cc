#include "state.hh"
#include "../util/common.hh"
#include "../util/util.hh"


State::State(Log& log)
    : log_(log)
{
    // sedentary
    this->colors_ = 0;
    // transportable
    this->num_ = 4000;
    this->width_ = 1000;
    this->height_ = 1000;
    this->alpha_ = PI; // 180 degrees
    this->beta_ = 0.296705972839036L; // 17 degrees
    this->scope_ = 24.0f;
    this->speed_ = 4.0f;
    // derived
    this->scope_squared_ = this->scope_ * this->scope_;

    this->spawn();
}


// spawn: Initialise particles.

void
State::spawn()
{
    unsigned int w = this->width_;
    unsigned int h = this->height_;
    float r = 2.0f; // TODO: particle radius fixed
    for (int i = 0; i < this->num_; ++i) {
        this->px_.push_back(Util::distribute<float>(0.0f,
                                                    static_cast<float>(w)));
        this->py_.push_back(Util::distribute<float>(0.0f,
                                                    static_cast<float>(h)));
        this->pf_.push_back(Util::distribute<float>(0.0f, TAU));
        this->pc_.push_back(cosf(this->pf_[i]));
        this->ps_.push_back(sinf(this->pf_[i]));
        this->pn_.push_back(0);
        this->pl_.push_back(0);
        this->pr_.push_back(0);
        this->prad_.push_back(r);
        this->pgcol_.push_back(0);
        this->pgrow_.push_back(0);
    }
}


// respawn: Reinitialise particles.

inline void
State::respawn()
{
    this->clear();
    this->spawn();
}


// clear: Clear out particles.

void
State::clear()
{
    this->px_.clear();
    this->py_.clear();
    this->pf_.clear();
    this->pc_.clear();
    this->ps_.clear();
    this->pn_.clear();
    this->pl_.clear();
    this->pr_.clear();
    this->prad_.clear();
    this->pgcol_.clear();
    this->pgrow_.clear();
}


// change: Mutate world.

bool
State::change(Stative& gui)
{
    bool respawn = false;

    if (gui.width  != this->width_  ||
        gui.height != this->height_ ||
        gui.num    != this->num_) {
        respawn = true;
    }

    if (!respawn &&
        gui.alpha  == this->alpha_ &&
        gui.beta   == this->beta_  &&
        gui.scope  == this->scope_ &&
        gui.speed  == this->speed_ &&
        gui.colors == this->colors_) {
        return false;
    }

    this->num_    = gui.num;
    this->width_  = gui.width;
    this->height_ = gui.height;
    this->alpha_  = gui.alpha;
    this->beta_   = gui.beta;
    this->scope_  = gui.scope;
    this->speed_  = gui.speed;
    this->colors_ = gui.colors;

    this->scope_squared_ = gui.scope * gui.scope;

    std::string message = "Changing state";
    if (respawn) {
        message += " and respawning.";
        this->respawn();
    } else {
        message += " without respawn.";
    }
    this->log_.add(Attn::O, message);

    this->notify(Topic::StateChanged); // Canvas reacts
    return true;
}

