#include "common.hh"
#include "log.hh"
#include "observation.hh"
#include "util.hh"


// common

TEST_CASE("util/common")
{
  std::string me = ME;
  std::string glsl_version = GLSL_VERSION;
  REQUIRE(0 == std::string(ME).compare("Emergence"));
  REQUIRE(0 == std::string(GLSL_VERSION).compare("#version 330 core"));
  REQUIRE(Approx(Util::deg_to_rad(180)) == M_PI);
  REQUIRE(Approx(Util::deg_to_rad(360)) == TAU);
}


// log

TEST_CASE("Log::Log")
{
  Log log = Log(1, true);
  REQUIRE(0 == log.messages_.size());
}

TEST_CASE("Log::add")
{
  Log log = Log(2, true);
  log.add(Attn::E, "bar", false);
  std::pair<Attn,std::string> message = log.messages_.front();
  REQUIRE(Attn::E == message.first);
  REQUIRE("Error: bar" == message.second);
  log.add(Attn::O, "foo", false);
  message = log.messages_.front();
  REQUIRE(Attn::O == message.first);
  REQUIRE("foo" == message.second);
  log.add(Attn::O, "baz", false);
  message = log.messages_.front();
  REQUIRE(Attn::O == message.first);
  REQUIRE("baz" == message.second);
  message = log.messages_.back();
  REQUIRE(Attn::O == message.first);
  REQUIRE("foo" == message.second);
}


// observation

class TestSubject : public Subject
{
  public:
    TestSubject() = default;
    void foo() { this->notify(Issue::StateChanged); }
    void bar() { this->notify(Issue::ProcDone); }
    void baz() { this->notify(Issue::NewMessage); }
};

class TestObserver : public Observer
{
  public:
    TestObserver(TestSubject &sub) : sub_(sub), val_(0) { sub.attach(*this); }
    ~TestObserver() override { this->sub_.detach(*this); }
    void react(Issue issue) override {
      if  (Issue::StateChanged == issue) { this->val_ = 1; }
      else if (Issue::ProcDone == issue) { this->val_ = 2; }
      else {                               this->val_ = 3; }
    }
    TestSubject& sub_;
    int val_;
};

TEST_CASE("observation cycle")
{
  TestSubject sub = TestSubject();
  TestObserver obs = TestObserver(sub);
  REQUIRE(0 == obs.val_);
  sub.foo();
  REQUIRE(1 == obs.val_);
  sub.bar();
  REQUIRE(2 == obs.val_);
  sub.baz();
  REQUIRE(3 == obs.val_);
}


// math

TEST_CASE("Util::deg_to_rad")
{
  REQUIRE(Approx(1.047198f) == Util::deg_to_rad(60));
  REQUIRE(Approx(1.570796f) == Util::deg_to_rad(90));
  REQUIRE(Approx(3.141593f) == Util::deg_to_rad(180));
  REQUIRE(Approx(6.283185f) == Util::deg_to_rad(360));
}

TEST_CASE("Util::rad_to_deg")
{
  REQUIRE(Approx(60) == Util::rad_to_deg(1.047198f));
  REQUIRE(Approx(90) == Util::rad_to_deg(1.570796f));
  REQUIRE(Approx(180) == Util::rad_to_deg(3.14159f));
  REQUIRE(Approx(360) == Util::rad_to_deg(6.283185f));
}

TEST_CASE("Util::signum")
{
  REQUIRE(-1 == Util::signum(-5));
  REQUIRE(0 == Util::signum(-0));
  REQUIRE(0 == Util::signum(0));
  REQUIRE(0 == Util::signum(-3 + 3));
  REQUIRE(0 == Util::signum(3 - 3));
  REQUIRE(1 == Util::signum(2));
}

// string

TEST_CASE("Util::trim")
{
  REQUIRE("foo" == Util::trim(" foo"));
  REQUIRE("foo" == Util::trim("foo "));
  REQUIRE("foo" == Util::trim(" foo "));
  REQUIRE("foo" == Util::trim("foo\t"));
  REQUIRE("foo" == Util::trim("foo\n"));
}

