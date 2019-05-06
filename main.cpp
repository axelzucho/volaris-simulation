#include <iostream>
#include <random>
#include "Simulation.h"

void do_experiment(int days){
    std::random_device generator;
    std::mt19937 mt(generator());
    Simulation simulation(mt, days, "../influences.txt");
    simulation.simulate();

    cout << "The results for the sales without any experiments are:\n";
    simulation.print_result_for_index(0);

    vector<int> best_indexes = simulation.get_best_conversions();
    cout << "The results for the sales with the experiment that yields to the highest sales are:\n";
    for(auto it: best_indexes){
        simulation.print_result_for_index(it);
    }

    simulation.write_all_results_to_file("../results.csv");

    //double sale_difference = simulation.all_averages[best_index].second[4] - simulation.all_averages[0].second[4];
    //cout << "The sale difference between doing and not doing this experiment is: " << sale_difference << " per hour!";
    //cout << "This would mean an increase of " << sale_difference*5000*24*365 << " pesos a year.\n";
}

int main() {
    do_experiment(36500);
    return 0;
}