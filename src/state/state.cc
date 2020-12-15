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


inline void
State::respawn()
{
    this->clear();
    this->spawn();
}


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


bool
State::change(Stative& input)
{
    bool respawn = false;

    if (input.width  != this->width_  ||
        input.height != this->height_ ||
        input.num    != this->num_) {
        respawn = true;
    }

    if (!respawn &&
        input.alpha  == this->alpha_ &&
        input.beta   == this->beta_  &&
        input.scope  == this->scope_ &&
        input.speed  == this->speed_ &&
        input.colors == this->colors_) {
        return false;
    }

    this->num_    = input.num;
    this->width_  = input.width;
    this->height_ = input.height;
    this->alpha_  = input.alpha;
    this->beta_   = input.beta;
    this->scope_  = input.scope;
    this->speed_  = input.speed;
    this->colors_ = input.colors;

    this->scope_squared_ = input.scope * input.scope;

    std::string message = "Changing state";
    if (respawn) {
        message += " and respawning.";
        this->respawn();
    } else {
        message += " without respawn.";
    }
    this->log_.add(Attn::O, message);

    this->notify(Issue::StateChanged); // Canvas reacts
    return true;
}

