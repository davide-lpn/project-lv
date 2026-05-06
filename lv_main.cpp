#include <ctime>
#include <string>

#include "lv.hpp"

static std::string timestamp() {
  std::time_t t = std::time(nullptr);
  char buf[20];
  std::strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", std::localtime(&t));
  return std::string(buf);
}

int main() {
  try {
    lv::Simulation s;
    s.compute();

    std::string dir = "results_" + timestamp();

    if (system(("mkdir -p " + dir).c_str()) != 0) {
      throw std::runtime_error{"Cannot create output directory"};
    }

    std::cout << "\nSimulation complete.\n";
    std::cout << "What would you like to do?\n";
    std::cout << "  [1] Print evolution to screen\n";
    std::cout << "  [2] Save evolution to file\n";
    std::cout << "  [3] Print statistics\n";
    std::cout << "  [4] Save statistics to file\n";
    std::cout << "  [5] Generate evolution plot\n";
    std::cout << "  [6] Generate phase space plot\n";
    std::cout << "  [7] Do all of the above\n";
    std::cout << "Enter choice: ";

    int choice;
    std::cin >> choice;

    switch (choice) {
      case 1:
        s.print_evolution();
        break;
      case 2:
        s.save_evolution(dir + "/evolution.txt");
        std::cout << "Saved to: " << dir << "/evolution.txt\n";
        break;
      case 3:
        s.print_statistics();
        break;
      case 4:
        s.save_statistics(dir + "/statistics.txt");
        std::cout << "Saved to: " << dir << "/statistics.txt\n";
        break;
      case 5:
        s.plot_evolution(dir + "/plot_evolution.png");
        std::cout << "Plot saved to: " << dir << "/plot_evolution.png\n";
        break;
      case 6:
        s.plot_phase_space(dir + "/plot_phase_space.png");
        std::cout << "Phase space plot saved to: " << dir << "/plot_phase_space.png\n";
        break;
      case 7:
        s.print_evolution();
        s.save_evolution(dir + "/evolution.txt");
        s.print_statistics();
        s.save_statistics(dir + "/statistics.txt");
        s.plot_evolution(dir + "/plot_evolution.png");
        s.plot_phase_space(dir + "/plot_phase_space.png");
        std::cout << "All files saved in: " << dir << "/\n";
        break;
      default:
        std::cerr << "Invalid choice\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

  } catch (std::invalid_argument const& e) {
    std::cerr << "Invalid argument: " << e.what() << '\n';
    return EXIT_FAILURE;
  } catch (std::runtime_error const& e) {
    std::cerr << "Runtime error: " << e.what() << '\n';
    return EXIT_FAILURE;
  } catch (...) {
    std::cerr << "Unknown error\n";
    return EXIT_FAILURE;
  }
}