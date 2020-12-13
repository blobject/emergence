#include "../src/util/util.hh"

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
  REQUIRE(0 == Util::signum(0));
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

