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

struct Statistics {
  double mean;
  double sigma;
  double maximum;
  double minimum;
};

class Simulation {
  Parameters const par_;
  double const dt_;
  std::size_t const steps_;
  State state_;
  std::vector<State> evolution_;
  Statistics sheep_stats_;
  Statistics wolf_stats_;

  explicit Simulation(Parameters par, double sheep, double wolf, double dt,
                      std::size_t steps);
  static Parameters const& is_valid(Parameters const& p);
  static double is_positive(double val);
  static double check_dt(double dt);
  static std::size_t to_steps(double duration, double dt);

 public:
  Simulation(Parameters p, double sheep, double wolf, double dt,
             double duration)
      : Simulation(is_valid(p), is_positive(sheep), is_positive(wolf),
                   check_dt(dt),
                   to_steps(is_positive(duration), check_dt(dt))) {}

  Simulation();

  // getters
  Parameters const& parameters() const { return par_; }
  State const& init_state() const { return evolution_.front(); }
  State const& current_state() const { return evolution_.back(); }
  std::vector<State> const& evolution() const { return evolution_; }
  double dt() const { return dt_; }
  std::size_t steps() const { return steps_; }
  Statistics const& sheep_stats() const { return sheep_stats_; }
  Statistics const& wolf_stats() const { return wolf_stats_; }

  // calculation methods
  void evolve();
  void compute();
  void statistics();
  double delta_H() const;

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