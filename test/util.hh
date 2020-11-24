#include "../src/util/util.hh"

// io

TEST_CASE("Util/LoadState")
{
  REQUIRE(true);
}

TEST_CASE("Util/SaveState")
{
  REQUIRE(true);
}

// math

TEST_CASE("Util/DegToRad")
{
  REQUIRE(Approx(1.047198f) == Util::DegToRad(60));
  REQUIRE(Approx(1.570796f) == Util::DegToRad(90));
  REQUIRE(Approx(3.141593f) == Util::DegToRad(180));
  REQUIRE(Approx(6.283185f) == Util::DegToRad(360));
}

TEST_CASE("Util/RadToDeg")
{
  REQUIRE(Approx(60) == Util::RadToDeg(1.047198f));
  REQUIRE(Approx(90) == Util::RadToDeg(1.570796f));
  REQUIRE(Approx(180) == Util::RadToDeg(3.14159f));
  REQUIRE(Approx(360) == Util::RadToDeg(6.283185f));
}

TEST_CASE("Util/Signum")
{
  REQUIRE(-1 == Util::Signum(-5));
  REQUIRE(0 == Util::Signum(0));
  REQUIRE(1 == Util::Signum(2));
}

// string

TEST_CASE("Util/Trim")
{
  REQUIRE("foo" == Util::Trim(" foo"));
  REQUIRE("foo" == Util::Trim("foo "));
  REQUIRE("foo" == Util::Trim(" foo "));
  REQUIRE("foo" == Util::Trim("foo\t"));
  REQUIRE("foo" == Util::Trim("foo\n"));
}

