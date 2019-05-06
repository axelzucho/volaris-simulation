//
// Created by axelzucho on 15/04/19.
//

#ifndef VOLARIS_SIMULATION_SIMULATION_H
#define VOLARIS_SIMULATION_SIMULATION_H

#include <map>
#include <random>
#include <vector>
#include <fstream>

using namespace std;

class Simulation {
private:
    int current_hour;
    int current_day;
    int amount_days;
    std::mt19937 mt;
    // First: step
    // Second: Day
    // Third: time
    vector<vector<double>> influence;
    vector<double> current_influence;
    map<tuple<int, int, int>, normal_distribution<double>> all_distributions;
    fstream influence_file;
public:
    double get_all_current_influence(int step);
    void populate_influence();
    vector<vector<int>> all_results;
    vector<pair<vector<double>, vector<double>>> all_averages;
    explicit Simulation(std::mt19937 mt, int amount_days, string influence_file);
    void populate_distributions(string filename);
    void next_hour();
    void simulate_day();
    vector<double> get_averages();
    // TODO (axelzucho): Add other iteration logic.
    bool next_influence();
    void flush_averages();
    void simulate();
    void simulate_iteration();
    bool next_iteration();
    vector<int> get_best_conversions();
    void print_result_for_index(int index);
    void write_all_results_to_file(string filename);
};


#endif //VOLARIS_SIMULATION_SIMULATION_H
