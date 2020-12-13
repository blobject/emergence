#pragma once

#include "cl.hh"
#include "../util/log.hh"


class Cl;
class State;


// Proc: Implementation of the particle system's core algorithm.

class Proc : public Subject
{
  public:
    Proc(Log& log, State& state, Cl& cl);
    void next();

    // done: Pause the system and notify Views
    inline void
    done()
    {
        this->paused_ = true;
        this->notify(Issue::ProcDone); // Views react
    }

    State& state_;
    bool   paused_;
    bool   cl_good_;

  private:
    Log&             log_;
    Cl&              cl_;
    std::vector<int> grid_;
    int              grid_cols_;
    int              grid_rows_;
    unsigned int     grid_stride_; // (max) size of a flattened grid unit

    void plot();
    void seek();
    void move();
    void plain_seek();
    void plain_seek_vicinity(std::vector<int>& grid, unsigned int stride,
                             int col, int row, int cols, int rows,
                             unsigned int srci);
    void plain_seek_tally(unsigned int srci, unsigned int dsti,
                          bool cunder, bool cover, bool runder, bool rover);
    void plain_move();
};

