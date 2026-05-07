#include "lv.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <numeric>
#include <stdexcept>

namespace {
double read_double(std::string const& prompt) {
  double val;
  std::cout << prompt;
  if (!(std::cin >> val)) {
    throw std::invalid_argument{"Invalid input: expected a number"};
  }
  return val;
}
}  // namespace

namespace lv {

Parameters const& Simulation::is_valid(Parameters const& p) {
  if (!((p.A > 0) && (p.B > 0) && (p.C > 0) && (p.D > 0))) {
    throw std::invalid_argument{"Parameters must be strictly greater than zero"};
  }
  return p;
}

double Simulation::is_positive(double val) {
  if (!(val > 0)) {
    throw std::invalid_argument{"Value must be strictly positive"};
  }
  return val;
}

double Simulation::check_dt(double dt) {
  if (!(dt > 0)) {
    throw std::invalid_argument{"Time step must be strictly positive"};
  }
  if (!(dt <= 0.001)) {
    std::cerr << "Warning: large time step reduces numerical stability\n";
  }
  return dt;
}

Parameters Simulation::read_parameters() {
  Parameters p;
  p.A = read_double("Enter sheep birth rate (A): ");
  p.B = read_double("Enter sheep death rate (B): ");
  p.C = read_double("Enter wolf birth rate (C): ");
  p.D = read_double("Enter wolf death rate (D): ");
  return p;
}

double Simulation::read_population(std::string const& prompt) { return read_double(prompt); }

double Simulation::read_dt() { return read_double("Enter time step (dt, recommended <= 0.001): "); }

double Simulation::read_duration() { return read_double("Enter simulation duration: "); }

Simulation::Simulation(Parameters par, double sheep, double wolf, double dt, double duration)
    : par_{is_valid(par)},
      dt_{check_dt(dt)},
      duration_{is_positive(duration)},
      iterations_{static_cast<std::size_t>(duration / dt)} {
  sheep = is_positive(sheep);
  wolf = is_positive(wolf);
  double H0 = (par_.C * sheep) + (par_.B * wolf) - (par_.D * std::log(sheep)) - (par_.A * std::log(wolf));
  evolution_.push_back(State{sheep, wolf, H0});
  state_.sheep = sheep * par_.C / par_.D;
  state_.wolf = wolf * par_.B / par_.A;
}

Simulation::Simulation()
    : Simulation(read_parameters(), read_population("Enter initial sheep population: "),
                 read_population("Enter initial wolf population: "), read_dt(), read_duration()) {}

void Simulation::evolve() {
  auto x = state_.sheep;
  auto y = state_.wolf;

  state_.sheep = x + par_.A * (1. - y) * x * dt_;
  state_.wolf = y + par_.D * (x - 1.) * y * dt_;

  if (!(state_.sheep > 0) || !(state_.wolf > 0)) {
    state_.sheep = x;
    state_.wolf = y;
    throw std::runtime_error{"Populations collapsed to zero"};
  }

  double x_new = state_.sheep * par_.D / par_.C;
  double y_new = state_.wolf * par_.A / par_.B;
  double H_new = (par_.C * x_new) + (par_.B * y_new) - (par_.D * std::log(x_new)) - (par_.A * std::log(y_new));

  evolution_.push_back(State{x_new, y_new, H_new});
}

void Simulation::compute() {
  for (; evolution_.size() < iterations_ + 1;) {
    try {
      evolve();
    } catch (std::runtime_error const& e) {
      std::cerr << e.what() << '\n';
      break;
    }
  }
}

double Simulation::delta_H() const {
  auto min_H =
      std::min_element(evolution_.begin(), evolution_.end(), [](State const& a, State const& b) { return a.H < b.H; });
  auto max_H =
      std::max_element(evolution_.begin(), evolution_.end(), [](State const& a, State const& b) { return a.H < b.H; });
  return max_H->H - min_H->H;
}

void Simulation::statistics() {
  if (stats_computed_ == true) {
    return;
  }

  auto const n = static_cast<double>(evolution_.size());

  double sheep_sum = std::accumulate(evolution_.begin(), evolution_.end(), 0.,
                                     [](double acc, State const& s) { return acc + s.sheep; });

  sheep_stats_.mean = sheep_sum / n;

  double sheep_sq_sum = std::accumulate(evolution_.begin(), evolution_.end(), 0., [&](double acc, State const& s) {
    return acc + (s.sheep - sheep_stats_.mean) * (s.sheep - sheep_stats_.mean);
  });

  sheep_stats_.sigma = std::sqrt(sheep_sq_sum / n);

  auto sheep_max = std::max_element(evolution_.begin(), evolution_.end(),
                                    [](State const& a, State const& b) { return a.sheep < b.sheep; });

  auto sheep_min = std::min_element(evolution_.begin(), evolution_.end(),
                                    [](State const& a, State const& b) { return a.sheep < b.sheep; });

  sheep_stats_.maximum = sheep_max->sheep;
  sheep_stats_.minimum = sheep_min->sheep;

  //

  double wolf_sum = std::accumulate(evolution_.begin(), evolution_.end(), 0.,
                                    [](double acc, State const& s) { return acc + s.wolf; });

  wolf_stats_.mean = wolf_sum / n;

  double wolf_sq_sum = std::accumulate(evolution_.begin(), evolution_.end(), 0., [&](double acc, State const& s) {
    return acc + (s.wolf - wolf_stats_.mean) * (s.wolf - wolf_stats_.mean);
  });

  wolf_stats_.sigma = std::sqrt(wolf_sq_sum / n);

  auto wolf_max = std::max_element(evolution_.begin(), evolution_.end(),
                                   [](State const& a, State const& b) { return a.wolf < b.wolf; });

  auto wolf_min = std::min_element(evolution_.begin(), evolution_.end(),
                                   [](State const& a, State const& b) { return a.wolf < b.wolf; });

  wolf_stats_.maximum = wolf_max->wolf;
  wolf_stats_.minimum = wolf_min->wolf;

  stats_computed_ = true;
}

void Simulation::print_evolution() const {
  std::cout << std::fixed << std::setprecision(6);
  std::cout << "time\t\tsheep\t\twolf\t\tH\n";

  double time = 0.;
  for (auto const& state : evolution_) {
    std::cout << time << '\t' << state.sheep << '\t' << state.wolf << '\t' << state.H << '\n';
    time += dt_;
  }
}

void Simulation::save_evolution(std::string const& filename) const {
  std::ofstream file{filename};
  if (!file) {
    throw std::runtime_error{"Cannot open file: " + filename};
  }

  file << std::fixed << std::setprecision(6);
  file << "time\t\tsheep\t\twolf\t\tH\n";

  double time = 0.;
  for (auto const& state : evolution_) {
    file << time << '\t' << state.sheep << '\t' << state.wolf << '\t' << state.H << '\n';
    time += dt_;
  }
}

void Simulation::print_statistics() {
  statistics();
  std::cout << std::fixed << std::setprecision(6);
  std::cout << "Simulation parameters:\n"
            << "  duration:   " << duration_ << '\n'
            << "  dt:         " << dt_ << '\n'
            << "  iterations: " << iterations() << '\n';
  std::cout << std::fixed << std::setprecision(6);
  std::cout << "Sheep statistics:\n"
            << " mean: " << sheep_stats_.mean << '\n'
            << " sigma: " << sheep_stats_.sigma << '\n'
            << "  maximum: " << sheep_stats_.maximum << '\n'
            << "  minimum: " << sheep_stats_.minimum << '\n';

  std::cout << "Wolf statistics:\n"
            << "  mean:    " << wolf_stats_.mean << '\n'
            << "  sigma:   " << wolf_stats_.sigma << '\n'
            << "  maximum: " << wolf_stats_.maximum << '\n'
            << "  minimum: " << wolf_stats_.minimum << '\n';
  std::cout << "Delta H: " << delta_H() << '\n';
}

void Simulation::save_statistics(std::string const& filename) {
  statistics();

  std::ofstream file{filename};
  if (!file) {
    throw std::runtime_error{"Cannot open file: " + filename};
  }

  file << std::fixed << std::setprecision(6);
  file << "Simulation parameters:\n"
       << "  duration:   " << duration_ << '\n'
       << "  dt:         " << dt_ << '\n'
       << "  iterations: " << iterations() << '\n';

  file << "Sheep statistics:\n"
       << " mean: " << sheep_stats_.mean << '\n'
       << " sigma: " << sheep_stats_.sigma << '\n'
       << "  maximum: " << sheep_stats_.maximum << '\n'
       << "  minimum: " << sheep_stats_.minimum << '\n';

  file << "Wolf statistics:\n"
       << "  mean:    " << wolf_stats_.mean << '\n'
       << "  sigma:   " << wolf_stats_.sigma << '\n'
       << "  maximum: " << wolf_stats_.maximum << '\n'
       << "  minimum: " << wolf_stats_.minimum << '\n';

  file << "Delta H: " << delta_H() << '\n';
}

void Simulation::plot_evolution(std::string const& filename) const {
  // writing data on a temporary CSV file
  std::ofstream data{".tmp_data.csv"};
  if (!data) {
    throw std::runtime_error{"Cannot create temporary data file"};
  }

  data << std::fixed << std::setprecision(6);
  double time = 0.;
  for (auto const& state : evolution_) {
    data << time << '\t' << state.sheep << '\t' << state.wolf << '\t' << state.H << '\n';
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

  if (system("rm .tmp_data.csv .tmp_script.gp") != 0) {
    std::cerr << "Warning: could not remove temporary files\n";
  }
}

void Simulation::plot_phase_space(std::string const& filename) const {
  // write sheep and wolf values to a temporary file (no time column needed)
  std::ofstream data{".tmp_phase.csv"};
  if (!data) {
    throw std::runtime_error{"Cannot create temporary data file"};
  }

  data << std::fixed << std::setprecision(6);
  for (auto const& state : evolution_) {
    data << state.sheep << '\t' << state.wolf << '\n';
  }
  data.close();

  // write the Gnuplot script: x axis = sheep, y axis = wolf
  std::ofstream script{".tmp_phase.gp"};
  if (!script) {
    throw std::runtime_error{"Cannot create temporary script file"};
  }

  script << "set terminal pngcairo size 800,800\n"
         << "set output '" << filename << "'\n"
         << "set xlabel 'sheep'\n"
         << "set ylabel 'wolf'\n"
         << "set title 'Lotka-Volterra: phase space orbit'\n"
         << "plot '.tmp_phase.csv' using 1:2 with lines title 'orbit'\n";
  script.close();

  if (system("gnuplot -persistent .tmp_phase.gp") != 0) {
    throw std::runtime_error{"Gnuplot execution failed"};
  }

  if (system("rm .tmp_phase.csv .tmp_phase.gp") != 0) {
    std::cerr << "Warning: could not remove temporary files\n";
  }
}

bool operator==(Parameters const& a, Parameters const& b) {
  return a.A == b.A && a.B == b.B && a.C == b.C && a.D == b.D;
}

bool operator==(State const& a, State const& b) { return a.sheep == b.sheep && a.wolf == b.wolf && a.H == b.H; }

}  // namespace lv