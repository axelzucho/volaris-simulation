//
// Created by axelzucho on 15/04/19.
//

#include "Simulation.h"
#include <iostream>

using namespace std;

double Simulation::get_all_current_influence(int step) {
    double result = 0;
    for (int i = 0; i < influence.size(); i++) {
        result += current_influence[i] * influence[i][step];
    }
    return result;
}

void Simulation::populate_distributions(string filepath) {
    all_distributions.clear();
    // Domingo 0, Lunes 1, martes 2, miercoles 3, jueves 4, viernes 5, sabado 6
    // 0: 0 - 8
    // 1: 9 - 13
    // 2: 14 - 17
    // 3: 18 - 24

    fstream file;
    file.open(filepath);
    if (!file.is_open()) {
        cout << "File not opened correctly" << endl;
        return;
    }

    string line;
    int step, hour, day;
    double mean, standard;
    file >> line;
    file >> line;
    file >> line;
    file >> line;
    file >> line;

    while (file >> step >> day >> hour >> mean >> standard) {
        if (step != 0) {
            standard = 0.01;
            mean += get_all_current_influence(step - 1);
        }
        all_distributions[make_tuple(step, day, hour)] = normal_distribution<double>(mean, standard);
    }

    file.close();
}

Simulation::Simulation(std::mt19937 mt, int amount_days, string influence_file) {
    this->mt = mt;
    all_results.clear();
    all_distributions.clear();
    this->amount_days = amount_days;
    current_day = 0;
    current_hour = 0;
    this->influence_file.open(influence_file);
    // Start with no influence.
    this->current_influence = {0, 0, 0, 0, 0, 0};
    if (!this->influence_file.is_open()) {
        printf("Unable to open influence file");
    }
    populate_influence();
}

tuple<int, int, int> get_current_indexes(int step, int day, int hour) {
    int index_hour;
    if (hour <= 8) index_hour = 0;
    else if (hour <= 13) index_hour = 1;
    else if (hour <= 17) index_hour = 2;
    else index_hour = 3;

    return make_tuple(step, day, index_hour);
}

void Simulation::next_hour() {
    tuple<int, int, int> current_indexes;
    vector<int> all_numbers;
    current_indexes = get_current_indexes(0, current_day, current_hour);
    int hour = get<2>(current_indexes);
    int minimum;
    if (hour == 0) {
        minimum = 1500;
    } else if (hour == 1) {
        minimum = 4500;
    } else if (hour == 2) {
        minimum = 4000;
    } else {
        minimum = 3500;
    }
    int visits = (int) all_distributions[current_indexes](mt);
    if (visits < minimum) visits = minimum;
    all_numbers.push_back(visits);
    for (int i = 0; i < 4; i++) {
        get<0>(current_indexes)++;
        double next_step = all_distributions[current_indexes](mt);
        if (next_step > 1) next_step = 1;
        else if (next_step < 0) next_step = 0;
        all_numbers.push_back((int) (all_numbers[i] * next_step));
    }

    current_hour++;
    if (current_hour >= 24) {
        current_day++;
        if(current_day > 6){
            current_day = 0;
        }
        current_hour = 0;
    }

    all_results.push_back(all_numbers);
}

void Simulation::simulate_day() {
    for (int i = 0; i < 24; ++i) {
        next_hour();
    }
}

vector<double> Simulation::get_averages() {
    vector<long long> total = {0, 0, 0, 0, 0};
    for (const auto &it:all_results) {
        for (int i = 0; i < it.size(); ++i) {
            total[i] += it[i];
        }
    }
    vector<double> result;
    for (const auto &it: total) {
        result.push_back((double) (it) / all_results.size());
    }

    return result;
}

void Simulation::populate_influence() {
    // Populating step reduction
    vector<double> step_reduction = {0, 0.006683792875, 0.0986736954, -0.002734292305};
    vector<double> loading_time = {0, 0.005127227114, 0.08601547609, -0.0004081311946};
    vector<double> targeting = {0, 0.001186207573, 0.04441355088, -0.0004291764325};
    vector<double> design = {0, 0.00124863955, 0.007431015711, -0.01091173244};
    vector<double> intuition = {0, 0.003272296752, 0.02717028809, -0.002343900366};
    vector<double> pressure = {0, 0.003514689104, 0.06199392569, -0.002508978267};

    influence.push_back(step_reduction);
    influence.push_back(loading_time);
    influence.push_back(targeting);
    influence.push_back(design);
    influence.push_back(intuition);
    influence.push_back(pressure);
}

bool Simulation::next_influence() {
    current_influence.clear();
    for (int i = 0; i < 6; i++) {
        float single_influence;
        if (!(influence_file >> single_influence)) {
            return false;
        }
        current_influence.push_back(single_influence);
    }
    return true;
}

void Simulation::flush_averages() {
    vector<double> averages = get_averages();
    all_results.clear();
    all_averages.push_back(make_pair(current_influence, averages));
}

void Simulation::simulate_iteration() {
    populate_distributions("../Example.tsv");
    for (int i = 0; i < amount_days; i++) {
        simulate_day();
    }
}

bool Simulation::next_iteration() {
    flush_averages();
    current_hour = 0;
    current_day = 0;
    bool cont = next_influence();
    return cont;
}

void Simulation::simulate() {
    int iterations = 0;
    bool cont = true;
    while (cont) {
        simulate_iteration();
        cont = next_iteration();
        if(++iterations % 30 == 0){
            cout << iterations/30 << "%\n";
        }
    }
}

vector<int> Simulation::get_best_conversions() {
    vector<int> best_indexes;
    int best_index = -1;
    double best_conversion = -1;
    for (int i = 0; i < all_averages.size(); i++) {
        if (best_conversion < all_averages[i].second[4]) {
            best_index = i;
            best_conversion = all_averages[i].second[4];
        }
    }
    for (int i = 0; i < all_averages.size(); i++) {
        if (best_conversion < (all_averages[i].second[4] + 1)) {
            best_indexes.push_back(i);
        }
    }
    return best_indexes;
}

void Simulation::print_result_for_index(int index) {
    cout << "The influences are:\n";
    cout << all_averages[index].first[0] << " of influence in step reduction.\n";
    cout << all_averages[index].first[1] << " of influence in loading time.\n";
    cout << all_averages[index].first[2] << " of influence in targeting.\n";
    cout << all_averages[index].first[3] << " of influence in design.\n";
    cout << all_averages[index].first[4] << " of influence in intuition.\n";
    cout << all_averages[index].first[5] << " of influence in pressure.\n";

    cout << "The average conversion per hour in each step is:\n";
    cout << all_averages[index].second[0] << " of people arrive to the web-page.\n";
    cout << all_averages[index].second[1] << " of people arrive to the flight search.\n";
    cout << all_averages[index].second[2] << " of people arrive to the view passengers.\n";
    cout << all_averages[index].second[3] << " of people arrive to the view payment.\n";
    cout << all_averages[index].second[4] << " of people arrive to the view confirmation.\n";
}

void Simulation::write_all_results_to_file(string filename) {
    ofstream file;
    file.open(filename);
    if(!file.is_open()){
        cout << "Can't open\n";
        return;
    }
    file << "step_reduction,loading_time,targeting,design,intuition,pressure,web_page,flight_search,view_passengers,view_payment,view_confirmation\n";
    for(const auto &it: this->all_averages){
        for (const auto &influences:it.first){
            file << influences << ",";
        }
        for (int i = 0; i < it.second.size() - 1; i++){
            file << it.second[i] << ",";
        }
        file << it.second[it.second.size()-1] << "\n";
    }
    file.close();

}