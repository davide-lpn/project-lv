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

  explicit Simulation(Parameters par, double sheep, double wolf, double dt,
                      double duration);

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
  Simulation(Parameters p, double sheep, double wolf, double dt,
             double duration)
      : Simulation(is_valid(p), is_positive(sheep), is_positive(wolf),
                   check_dt(dt), is_positive(duration)) {}

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

void Simulation::plot_all(std::string const& filename) const {
  // writing data on a temporary CSV file
  std::ofstream data{".tmp_data.csv"};
  if (!data) {
    throw std::runtime_error{"Cannot create temporary data file"};
  }

  data << std::fixed << std::setprecision(6);
  double time = 0.;
  for (auto const& state : evolution_) {
    data << time << '\t' << state.sheep << '\t' << state.wolf << '\t' << state.H
         << '\n';
    time += dt_;
  }
  data.close();

  // Writing Gnuplot temporary script
  std::ofstream script{".tmp_script.gp"};
  if (!script) {
    throw std::runtime_error{"Cannot create temporary script file"};
  }

  script << "set terminal pngcairo size 1200,800\n"
         << "set output '" << filename << "'\n"
         << "set multiplot layout 2,1\n"
         << "set xlabel 'time'\n"
         << "set ylabel 'population'\n"
         << "set title 'Lotka-Volterra: Sheep and Wolf populations'\n"
         << "plot '.tmp_data.csv' using 1:2 with lines title 'sheep', "
         << "     '.tmp_data.csv' using 1:3 with lines title 'wolf'\n"
         << "set ylabel 'H'\n"
         << "set title 'First integral H'\n"
         << "plot '.tmp_data.csv' using 1:4 with lines title 'H'\n"
         << "unset multiplot\n";
  script.close();

  // execute Gnuplot
  if (system("gnuplot -persistent .tmp_script.gp") != 0) {
    throw std::runtime_error{"Gnuplot execution failed"};
  }

  system("rm .tmp_data.csv .tmp_script.gp");
}

void Simulation::plot_all(std::string const& filename) const {
  // scrive i dati su file CSV temporaneo
  std::ofstream data{".tmp_data.csv"};
  if (!data) {
    throw std::runtime_error{"Cannot create temporary data file"};
  }

  data << std::fixed << std::setprecision(6);
  double time = 0.;
  for (auto const& state : evolution_) {
    data << time << '\t' << state.sheep << '\t' << state.wolf << '\t' << state.H
         << '\n';
    time += dt_;
  }
  data.close();

  // scrive il copione Gnuplot
  std::ofstream script{".tmp_script.gp"};
  if (!script) {
    throw std::runtime_error{"Cannot create temporary script file"};
  }

  script << "set terminal pngcairo size 1200,800\n"
         << "set output '" << filename << "'\n"
         << "set multiplot layout 2,1\n"
         << "set xlabel 'time'\n"
         << "set ylabel 'population'\n"
         << "set title 'Lotka-Volterra: Sheep and Wolf populations'\n"
         << "plot '.tmp_data.csv' using 1:2 with lines title 'sheep', "
         << "     '.tmp_data.csv' using 1:3 with lines title 'wolf'\n"
         << "set ylabel 'H'\n"
         << "set title 'First integral H'\n"
         << "plot '.tmp_data.csv' using 1:4 with lines title 'H'\n"
         << "unset multiplot\n";
  script.close();

  // esegue Gnuplot
  if (system("gnuplot -persistent .tmp_script.gp") != 0) {
    throw std::runtime_error{"Gnuplot execution failed"};
  }

  // rimuove i file temporanei
  system("rm .tmp_data.csv .tmp_script.gp");
}

bool operator==(Parameters const& a, Parameters const& b);
bool operator==(State const& a, State const& b);

}  // namespace lv

#endif