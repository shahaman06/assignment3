#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <random>

using namespace std;

const vector<string> DAYS = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
const vector<string> SHIFTS = {"Morning", "Afternoon", "Evening"};

struct Employee {
    string name;
    map<string, vector<string>> preferences;  // day -> [shift1, shift2, ...]
    set<string> assigned_days;
};

map<string, map<string, vector<string>>> schedule;  // day -> shift -> list of names
vector<Employee> employees;
set<string> manager_assigned_days;

void read_csv(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening CSV file." << endl;
        exit(1);
    }

    string line;
    getline(file, line);  // header

    while (getline(file, line)) {
        stringstream ss(line);
        string cell;
        Employee emp;
        getline(ss, emp.name, ',');

        int i = 0;
        while (getline(ss, cell, ',') && i < DAYS.size()) {
            if (!cell.empty()) {
                vector<pair<int, string>> ranks;
                stringstream shift_stream(cell);
                string item;
                while (getline(shift_stream, item, '|')) {
                    auto pos = item.find(':');
                    if (pos != string::npos) {
                        int rank = stoi(item.substr(0, pos));
                        string shift = item.substr(pos + 1);
                        ranks.push_back({rank, shift});
                    }
                }
                sort(ranks.begin(), ranks.end());
                for (const auto& rank_pair : ranks) {
                    const auto& s = rank_pair.second;
                    emp.preferences[DAYS[i]].push_back(s);
                }
            }
            i++;
        }
        employees.push_back(emp);
    }
    file.close();
}

bool assign_employee(Employee& emp, const string& day, const string& shift) {
    if (emp.assigned_days.size() >= 5 || emp.assigned_days.count(day)) return false;
    schedule[day][shift].push_back(emp.name);
    emp.assigned_days.insert(day);
    return true;
}

void generate_schedule() {
    schedule.clear();
    manager_assigned_days.clear();

    for (auto& emp : employees)
        emp.assigned_days.clear();

    random_device rd;
    mt19937 g(rd());

    for (const string& day : DAYS) {
        for (const string& shift : SHIFTS) {
            int count = 0;

            // Step 1: Priority-based assignment
            for (auto& emp : employees) {
                auto& prefs = emp.preferences[day];
                if (find(prefs.begin(), prefs.end(), shift) != prefs.end()) {
                    if (assign_employee(emp, day, shift)) {
                        count++;
                        if (count == 2) break;
                    }
                }
            }

            // Step 2: Fill remaining with available employees
            while (count < 2) {
                vector<Employee*> candidates;
                for (auto& emp : employees) {
                    if (emp.assigned_days.size() < 5 && !emp.assigned_days.count(day)) {
                        candidates.push_back(&emp);
                    }
                }
                shuffle(candidates.begin(), candidates.end(), g);
                bool assigned = false;
                for (auto* emp : candidates) {
                    if (assign_employee(*emp, day, shift)) {
                        count++;
                        assigned = true;
                        break;
                    }
                }

                // Step 3: Use Manager
                if (!assigned && manager_assigned_days.size() < 5) {
                    schedule[day][shift].push_back("Manager");
                    manager_assigned_days.insert(day);
                    count++;
                    assigned = true;
                }

                // Step 4: Still can't assign
                if (!assigned) {
                    schedule[day][shift].push_back("Unassigned");
                    break;
                }
            }
        }
    }
}

void display_schedule() {
    vector<string> manager_only;
    vector<string> unassigned;

    for (const string& day : DAYS) {
        cout << day << ":\n";
        for (const string& shift : SHIFTS) {
            auto& names = schedule[day][shift];
            cout << "  " << shift << ": ";
            for (const auto& name : names) {
                cout << name << " ";
            }
            cout << "\n";

            if (names.size() == 1 && names[0] == "Manager") {
                manager_only.push_back(day + " - " + shift);
            }
            if (find(names.begin(), names.end(), "Unassigned") != names.end()) {
                unassigned.push_back(day + " - " + shift);
            }
        }
        cout << endl;
    }

    if (!manager_only.empty()) {
        cout << "\n⚠ Shifts covered only by Manager:\n";
        for (const auto& s : manager_only)
            cout << "• " << s << endl;
    }

    if (!unassigned.empty()) {
        cout << "\nUnassigned Shifts (no employees or manager available):\n";
        for (const auto& s : unassigned)
            cout << "• " << s << endl;
    }
}

int main() {
    string csv_path = "./errorless_schedule.csv";  // Adjust path if needed
    read_csv(csv_path);
    generate_schedule();
    display_schedule();
    return 0;
}
