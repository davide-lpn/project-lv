#ifndef LV_HPP
#define LV_HPP

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace lv {
struct Parameters {
  double A;
  double B;
  double C;
  double D;
};

struct State {
  double sheep;
  double wolf;
  double H;
};

// variables in relative form to the equilibrium point
struct StateRel {
  double sheep;
  double wolf;
};

struct Statistics {
  double mean;
  double sigma;
  double maximum;
  double minimum;
};

class Simulation {
  Parameters const par_;
  double const dt_;
  double const duration_;
  std::size_t const iterations_;
  StateRel state_;
  std::vector<State> evolution_;
  Statistics sheep_stats_;
  Statistics wolf_stats_;

  // validation
  static Parameters const& is_valid(Parameters const& p);
  static double is_positive(double val);
  static double check_dt(double dt);
  static std::size_t count_iterations(double duration, double dt);

  // input reading helper
  static Parameters read_parameters();
  static double read_population(std::string const& prompt);
  static double read_dt();
  static double read_duration();

 public:
  Simulation(Parameters p, double sheep, double wolf, double dt, double duration);

  Simulation();

  // getters
  Parameters const& parameters() const { return par_; }
  State const& init_state() const { return evolution_.front(); }
  State const& current_state() const { return evolution_.back(); }
  std::vector<State> const& evolution() const { return evolution_; }
  double dt() const { return dt_; }
  double duration() const { return duration_; }
  std::size_t iterations() const { return iterations_; }
  Statistics const& sheep_stats() const { return sheep_stats_; }
  Statistics const& wolf_stats() const { return wolf_stats_; }

  // calculation methods
  void evolve();
  void compute();
  double delta_H() const;
  void statistics();

  // output methods
  void print_evolution() const;
  void save_evolution(std::string const& filename) const;
  void print_statistics();
  void save_statistics(std::string const& filename);
  void plot_all(std::string const& filename) const;
};

bool operator==(Parameters const& a, Parameters const& b);
bool operator==(State const& a, State const& b);

}  // namespace lv

#endif