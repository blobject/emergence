#include "state.hh"
#include "../util/util.hh"


Particle::Particle(Distribution distribution,
                   unsigned int width,
                   unsigned int height)
{
  this->size_ = 5.0f;
  this->speed_ = 0.0f;
  this->neighsize_ = 0.0f;
  if (distribution == Distribution::UNIFORM)
  {
    this->x_ = Util::Distribute<float>(0.0f, (float) width - 1);
    this->y_ = Util::Distribute<float>(0.0f, (float) height - 1);
    this->phi_ = Util::Distribute<float>(0.0f, 1.0f);
  }
}


History::History()
{
  this->snapshots_ = std::vector<int>(100, 0);
}


State::State(const std::string &load)
{
  // sedentary data
  this->history_ = History();
  this->colorscheme_ = 0;

  // transportable data
  this->width_ = 800;
  this->height_ = 800;
  this->alpha_ = 0.0;
  this->beta_ = 0.0;
  this->gamma_ = 0.0;
  this->distribution_ = Distribution::UNIFORM;
  this->stop_ = 0;
  for (int i = 0; i < 1000; ++i)
  {
    this->particles_.push_back(Particle(this->distribution_,
                                        this->width_, this->height_));
  }

  if (! load.empty())
  {
    if (! Util::LoadState(*this, load))
    {
      Util::Err("could not load file '" + load + "'");
    }
  }
}

//State::~State()
//{
//}

