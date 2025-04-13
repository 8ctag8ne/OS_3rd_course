#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <random>
#include <chrono>

using namespace std;

const int NUM_PHILOSOPHERS = 5;
mutex forks[NUM_PHILOSOPHERS];
mutex eat_counter_mutex;
random_device rd;
mt19937 gen(rd());
uniform_int_distribution<int> dist(1000, 5000);
int eat_counter[NUM_PHILOSOPHERS] = {0};

class Philosopher {
private:
    int id;
    int left_fork;
    int right_fork;

public:
    Philosopher(int id) : id(id), left_fork(id), right_fork((id + 1) % NUM_PHILOSOPHERS) {
        if (id % 2 == 0) swap(left_fork, right_fork);
    }

    void think() {
        cout << "Philosopher " << id << " is thinking...\n";
        this_thread::sleep_for(chrono::milliseconds(dist(gen)));
    }

    bool try_pick_up_fork(int fork) {
        return forks[fork].try_lock();
    }

    void put_down_fork(int fork) {
        forks[fork].unlock();
    }

    void eat() {
        if (!try_pick_up_fork(left_fork)) {
            return; // Не вдалося взяти ліву вилку, повертаємось до роздумів
        }
        cout << "Philosopher " << id << " picked up left fork " << left_fork << "\n";

        if (!try_pick_up_fork(right_fork)) {
            cout << "Philosopher " << id << " couldn't pick up right fork " << right_fork << " and puts down left fork\n";
            put_down_fork(left_fork);
            return; // Не вдалося взяти праву вилку, кладемо ліву назад
        }

        cout << "Philosopher " << id << " picked up right fork " << right_fork << " and starts eating\n";
        this_thread::sleep_for(chrono::milliseconds(dist(gen)));

        cout << "Philosopher " << id << " finished eating and puts down forks\n";
        put_down_fork(right_fork);
        put_down_fork(left_fork);

        {
            lock_guard<mutex> lock(eat_counter_mutex);
            eat_counter[id]++;
        }
    }

    void dine() {
        while (true) {
            think();
            eat();
        }
    }
};

void monitor() {
    while (true) {
        this_thread::sleep_for(chrono::seconds(1));
        lock_guard<mutex> lock(eat_counter_mutex);
        bool all_ate = true;
        for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
            if (eat_counter[i] == 0) {
                all_ate = false;
                break;
            }
        }
        if (all_ate) {
            cout << "All philosophers have eaten at least once. Resetting counters.\n";
            fill(begin(eat_counter), end(eat_counter), 0);
        }
    }
}

int main() {
    vector<thread> philosophers;
    vector<Philosopher> philosopher_objs;

    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        philosopher_objs.emplace_back(i);
    }

    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        philosophers.emplace_back(&Philosopher::dine, &philosopher_objs[i]);
    }

    thread monitor_thread(monitor);

    for (auto &p : philosophers) {
        p.join();
    }
    
    monitor_thread.join();

    return 0;
}
