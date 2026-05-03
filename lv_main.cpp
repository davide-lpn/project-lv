#include "lv.hpp"

int main() {
  try {
    lv::Simulation s;
    s.compute();

    std::cout << "\nSimulation complete.\n";
    std::cout << "What would you like to do?\n";
    std::cout << "  [1] Print evolution to screen\n";
    std::cout << "  [2] Save evolution to file\n";
    std::cout << "  [3] Print statistics\n";
    std::cout << "  [4] Save statistics to file\n";
    std::cout << "  [5] Generate plot\n";
    std::cout << "  [6] Do all of the above\n";
    std::cout << "Enter choice: ";

    int choice;
    std::cin >> choice;

    std::string filename;

    switch (choice) {
      case 1:
        s.print_evolution();
        break;
      case 2:
        std::cout << "Enter output filename: ";
        std::cin >> filename;
        s.save_evolution(filename);
        break;
      case 3:
        s.print_statistics();
        break;
      case 4:
        std::cout << "Enter output filename: ";
        std::cin >> filename;
        s.save_statistics(filename);
        break;
      case 5:
        std::cout << "Enter output filename (e.g. plot.png): ";
        std::cin >> filename;
        s.plot_all(filename);
        break;
      case 6:
        s.print_evolution();
        std::cout << "Enter filename for evolution: ";
        std::cin >> filename;
        s.save_evolution(filename);
        s.print_statistics();
        std::cout << "Enter filename for statistics: ";
        std::cin >> filename;
        s.save_statistics(filename);
        std::cout << "Enter filename for plot (e.g. plot.png): ";
        std::cin >> filename;
        s.plot_all(filename);
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