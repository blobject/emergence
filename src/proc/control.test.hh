#include "control.hh"
#include "../util/util.hh"
#include <stdio.h>


#define TESTFILE "testemergence.save"


void
rm_file(const std::string& name)
{
  // NOTE: deletes "testemergence.save"
  if (remove(name.c_str())) {
    std::cerr << "Failed to remove '" << name << "'" << std::endl;
  }
}


void
gen_file(const std::string& name, const std::string& body)
{
  // NOTE: overwrites "testemergence.save"
  std::fstream fs;
  fs.open(name, std::ios_base::out);
  if (!fs.is_open()) {
    std::cerr << "Failed to open '" << name << "'" << std::endl;
  } else {
    fs << body << std::endl;
  }
  fs.close();
}


std::vector<std::string>
split(const std::string& line)
{
  std::stringstream s(line);
  std::vector<std::string> words;
  std::string word;
  while (std::getline(s, word, ' ')) {
    words.push_back(word);
  }
  return words;
}


TEST_CASE("Control::change")
{
  auto log = Log(1, QUIET);
  auto expctrl = ExpControl(log, 0);
  auto state = State(log, expctrl);
  auto cl = Cl(log);
  auto proc = Proc(log, state, cl, false);
  auto exp = Exp(log, expctrl, state, proc, false);
  auto ctrl = Control(log, state, proc, expctrl, exp, "", false);
  REQUIRE(-1 == ctrl.countdown_);
  REQUIRE(5000 == state.num_);
  REQUIRE(250 == state.height_);
  REQUIRE(Approx(Util::deg_to_rad(180)) == state.alpha_);
  REQUIRE(0 == state.coloring_);
  Stative stative = {
    100,
    2000,
    state.width_,
    500,
    Util::deg_to_rad(60),
    state.beta_,
    state.scope_,
    state.ascope_,
    state.speed_,
    state.noise_,
    state.prad_,
    static_cast<int>(Coloring::Dynamic)
  };
  ctrl.change(stative, true);
  REQUIRE(100 == ctrl.countdown_);
  REQUIRE(2000 == state.num_);
  REQUIRE(500 == state.height_);
  REQUIRE(Approx(Util::deg_to_rad(60)) == state.alpha_);
  REQUIRE(1 == state.coloring_);
}


TEST_CASE("Control::load")
{
  std::string f = TESTFILE;
  gen_file(f, "100 400 500 60 30 10.5 2.3 1.1 12.34 3\n\
0 123.456 456.789 1.234\n\
1 42.42 19.17 359.999");
  auto log = Log(1, QUIET);
  auto expctrl = ExpControl(log, 0);
  auto state = State(log, expctrl);
  auto cl = Cl(log);
  auto proc = Proc(log, state, cl, false);
  auto exp = Exp(log, expctrl, state, proc, false);
  auto ctrl = Control(log, state, proc, expctrl, exp, "", false);
  REQUIRE(-1 == ctrl.countdown_);
  REQUIRE(5000 == state.num_);
  REQUIRE(0 == state.coloring_);
  ctrl.load(f);
  REQUIRE(100 == ctrl.countdown_);
  REQUIRE(2 == state.num_);
  REQUIRE(400 == state.width_);
  REQUIRE(500 == state.height_);
  REQUIRE(Approx(Util::deg_to_rad(60)) == state.alpha_);
  REQUIRE(Approx(Util::deg_to_rad(30)) == state.beta_);
  REQUIRE(Approx(10.5f) == state.scope_);
  REQUIRE(Approx(2.3f) == state.ascope_);
  REQUIRE(Approx(1.1f) == state.speed_);
  REQUIRE(Approx(Util::deg_to_rad(12.34f)) == state.noise_);
  REQUIRE(Approx(3.0f) == state.prad_);
  REQUIRE(0 == state.coloring_);
  rm_file(f);
}


TEST_CASE("Control::save")
{
  std::string f = TESTFILE;
  auto log = Log(1, QUIET);
  auto expctrl = ExpControl(log, 0);
  auto state = State(log, expctrl);
  auto cl = Cl(log);
  auto proc = Proc(log, state, cl, false);
  auto exp = Exp(log, expctrl, state, proc, false);
  auto ctrl = Control(log, state, proc, expctrl, exp, "", false);
  Stative stative = {
    ctrl.countdown_,
    2,
    state.width_,
    state.height_,
    state.alpha_,
    state.beta_,
    state.scope_,
    state.ascope_,
    state.speed_,
    state.noise_,
    state.prad_,
    static_cast<int>(state.coloring_)
  };
  ctrl.change(stative, true);
  REQUIRE(2 == state.num_);
  ctrl.save(f);
  std::vector<std::string> lines;
  std::ifstream fs(f);
  if (!fs) {
    std::cerr << "Failed to open '" << f << "'" << std::endl;
    return;
  }
  std::string line;
  while (std::getline(fs, line)) {
    lines.push_back(line);
  }
  fs.close();
  std::vector<std::string> words;
  REQUIRE(3 == lines.size());
  REQUIRE("-1 250 250 180 17 5 1.3 0.67 0 1" == lines[0]);
  words = split(lines[1]);
  REQUIRE(4 == words.size());
  REQUIRE("0" == words[0]);
  words = split(lines[2]);
  REQUIRE(4 == words.size());
  REQUIRE("1" == words[0]);
}

