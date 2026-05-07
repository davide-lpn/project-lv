#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "lv.hpp"

#include <algorithm>
#include <cmath>

#include "doctest.h"

static lv::Parameters const test_par{1.5, 0.5, 0.3, 1.2};
static double const test_sheep{8.};
static double const test_wolf{6.};
static double const test_dt{0.001};
static double const test_duration{1.};

TEST_CASE("Testing constructors") {
  SUBCASE("Valid construction does not throw") {
    bool ok = true;
    try {
      lv::Simulation s(test_par, test_sheep, test_wolf, test_dt, test_duration);
    } catch (...) {
      ok = false;
    }
    CHECK(ok);
  }

  SUBCASE("Negative parameter A throws") {
    CHECK_THROWS(lv::Simulation(lv::Parameters{-1.5, 0.5, 0.3, 1.2}, test_sheep, test_wolf, test_dt, test_duration));
  }

  SUBCASE("Zero parameter B throws") {
    CHECK_THROWS(lv::Simulation(lv::Parameters{1.5, 0., 0.3, 1.2}, test_sheep, test_wolf, test_dt, test_duration));
  }

  SUBCASE("Negative sheep throws") { CHECK_THROWS(lv::Simulation(test_par, -8., test_wolf, test_dt, test_duration)); }

  SUBCASE("Negative wolf throws") { CHECK_THROWS(lv::Simulation(test_par, test_sheep, -6., test_dt, test_duration)); }

  SUBCASE("Negative dt throws") {
    CHECK_THROWS(lv::Simulation(test_par, test_sheep, test_wolf, -0.001, test_duration));
  }

  SUBCASE("Zero dt throws") { CHECK_THROWS(lv::Simulation(test_par, test_sheep, test_wolf, 0., test_duration)); }

  SUBCASE("dt above 0.001 warns but does not throw") {
    bool ok = true;
    try {
      lv::Simulation s(test_par, test_sheep, test_wolf, 0.002, test_duration);
    } catch (...) {
      ok = false;
    }
    CHECK(ok);
  }

  SUBCASE("Zero duration throws") { CHECK_THROWS(lv::Simulation(test_par, test_sheep, test_wolf, test_dt, 0.)); }
}

TEST_CASE("Testing initial state") {
  lv::Simulation s(test_par, test_sheep, test_wolf, test_dt, test_duration);

  SUBCASE("Initial sheep is correct") { CHECK(s.init_state().sheep == doctest::Approx(test_sheep)); }

  SUBCASE("Initial wolf is correct") { CHECK(s.init_state().wolf == doctest::Approx(test_wolf)); }

  SUBCASE("Initial H is correct") {
    double expected_H = (test_par.C * test_sheep) + (test_par.B * test_wolf) - (test_par.D * std::log(test_sheep)) -
                        (test_par.A * std::log(test_wolf));
    CHECK(s.init_state().H == doctest::Approx(expected_H).epsilon(0.001));
  }
}

TEST_CASE("Testing evolve()") {
  lv::Simulation s(test_par, test_sheep, test_wolf, test_dt, test_duration);
  s.evolve();

  SUBCASE("sheep after one step") { CHECK(s.current_state().sheep == doctest::Approx(7.988).epsilon(0.001)); }

  SUBCASE("wolf after one step") { CHECK(s.current_state().wolf == doctest::Approx(6.007).epsilon(0.001)); }

  SUBCASE("evolution size after one step") { CHECK(s.evolution().size() == 2); }
}

TEST_CASE("Testing compute()") {
  lv::Simulation s(test_par, test_sheep, test_wolf, test_dt, test_duration);
  s.compute();

  SUBCASE("evolution size is iterations + 1") { CHECK(s.evolution().size() == s.iterations() + 1); }

  SUBCASE("first computed state matches evolve()") {
    CHECK(s.evolution()[1].sheep == doctest::Approx(7.988).epsilon(0.001));
    CHECK(s.evolution()[1].wolf == doctest::Approx(6.007).epsilon(0.001));
  }
}

TEST_CASE("Testing delta_H()") {
  SUBCASE("delta_H is small for small dt") {
    lv::Simulation s(test_par, test_sheep, test_wolf, test_dt, test_duration);
    s.compute();
    CHECK(s.delta_H() < 1.);
  }

  SUBCASE("larger dt produces larger delta_H") {
    lv::Simulation s_small(test_par, test_sheep, test_wolf, 0.0009, 10.);
    lv::Simulation s_large(test_par, test_sheep, test_wolf, 0.001, 10.);
    s_small.compute();
    s_large.compute();
    CHECK(s_small.delta_H() <= s_large.delta_H());
  }
}

TEST_CASE("Testing populations remain positive") {
  lv::Simulation s(test_par, test_sheep, test_wolf, test_dt, 10.);
  s.compute();
  auto const& ev = s.evolution();

  SUBCASE("sheep always positive") {
    bool ok = std::all_of(ev.begin(), ev.end(), [](lv::State const& st) { return st.sheep > 0.; });
    CHECK(ok);
  }

  SUBCASE("wolf always positive") {
    bool ok = std::all_of(ev.begin(), ev.end(), [](lv::State const& st) { return st.wolf > 0.; });
    CHECK(ok);
  }

  SUBCASE("H always positive") {
    bool ok = std::all_of(ev.begin(), ev.end(), [](lv::State const& st) { return st.H > 0.; });
    CHECK(ok);
  }
}

TEST_CASE("Testing single step simulation") {
  lv::Simulation s(test_par, test_sheep, test_wolf, test_dt, test_dt);
  s.compute();
  CHECK(s.evolution().size() == 2);
}

TEST_CASE("Testing getters") {
  lv::Simulation s(test_par, test_sheep, test_wolf, test_dt, test_duration);

  SUBCASE("parameters() returns the parameters passed at construction") { CHECK(s.parameters() == test_par); }

  SUBCASE("dt() returns the time step passed at construction") { CHECK(s.dt() == doctest::Approx(test_dt)); }

  SUBCASE("duration() returns the duration passed at construction") {
    CHECK(s.duration() == doctest::Approx(test_duration));
  }

  SUBCASE("iterations() matches duration / dt") {
    // 1.0 / 0.001 = 1000
    CHECK(s.iterations() == 1000);
  }

  SUBCASE("sheep_stats() and wolf_stats() are valid after compute() and statistics()") {
    s.compute();
    s.statistics();
    // means must be positive (populations are always positive)
    CHECK(s.sheep_stats().mean > 0.);
    CHECK(s.wolf_stats().mean > 0.);
    // maximum must be >= minimum
    CHECK(s.sheep_stats().maximum >= s.sheep_stats().minimum);
    CHECK(s.wolf_stats().maximum >= s.wolf_stats().minimum);
    // sigma must be non-negative
    CHECK(s.sheep_stats().sigma >= 0.);
    CHECK(s.wolf_stats().sigma >= 0.);
  }

  SUBCASE("calling statistics() twice gives same result") {
    s.compute();
    s.statistics();
    double mean_sheep_first = s.sheep_stats().mean;
    double mean_wolf_first = s.wolf_stats().mean;

    s.statistics();  // seconda chiamata
    CHECK(s.sheep_stats().mean == mean_sheep_first);
    CHECK(s.wolf_stats().mean == mean_wolf_first);
  }
}

TEST_CASE("Testing operator==") {
  SUBCASE("Parameters equal") {
    lv::Parameters a{1.5, 0.5, 0.3, 1.2};
    lv::Parameters b{1.5, 0.5, 0.3, 1.2};
    CHECK(a == b);
  }

  SUBCASE("Parameters not equal") {
    lv::Parameters a{1.5, 0.5, 0.3, 1.2};
    lv::Parameters b{1.5, 0.5, 0.3, 2.0};
    CHECK_FALSE(a == b);
  }

  SUBCASE("State equal") {
    lv::State a{8., 6., 0.217};
    lv::State b{8., 6., 0.217};
    CHECK(a == b);
  }

  SUBCASE("State not equal") {
    lv::State a{8., 6., 0.217};
    lv::State b{4., 6., 0.217};
    CHECK_FALSE(a == b);
  }
}