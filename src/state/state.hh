#pragma once

#include "../proc/control.hh"
#include "../util/log.hh"
#include <string>
#include <vector>


// Mutually include-dependent classes.

class Stative;


// State: Main data source of the primordial particle system.

class State : public Subject
{
  public:
    State(Log& log);
    void spawn();
    void respawn();
    void clear();
    bool change(Stative& gui);

    //// particle
    // location & direction
    std::vector<float> px_;        // volatile
    std::vector<float> py_;        // volatile
    std::vector<float> pf_;        // volatile
    std::vector<float> pc_;        // volatile
    std::vector<float> ps_;        // volatile
    // neighborhood
    std::vector<unsigned int> pn_; // volatile
    std::vector<unsigned int> pl_; // volatile
    std::vector<unsigned int> pr_; // volatile
    // visualisation
    std::vector<float> prad_;      // adjustable
    // grid
    std::vector<int> pgcol_;       // volatile
    std::vector<int> pgrow_;       // volatile

    // sedentary
    int colors_;                   // adjustable

    // transportable
    int          num_;    // # particles (negative for error)
    unsigned int width_;  // processable space width
    unsigned int height_; // processable space height
    float        alpha_;  // alpha in main formula (radians)
    float        beta_;   // beta in main formula (radians)
    float        scope_;  // neighborhood radius
    float        speed_;  // movement multiplier

    // derived
    float scope_squared_;

  private:
    Log& log_;
};

