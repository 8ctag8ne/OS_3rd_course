#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <vector>
#include <condition_variable>

// Функція для вимірювання часу виконання
double measureTime(std::chrono::time_point<std::chrono::high_resolution_clock> start) {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    return duration / 1000.0; // повертаємо час у секундах
}
// === 5. Синхронне паралельне додавання (bonus task) ===
void synchronizedThreads() {
    const int SYNC_ITERATIONS = 1000;
    std::atomic<int> sharedVariable(0);
    std::mutex mtx;
    std::condition_variable cv;
    bool threadTurn = true;  // Визначає чергу потоків
    
    // Тимчасові змінні для реалізації покрокового інкременту
    int tempValue1 = 0;
    int tempValue2 = 0;
    int turn1 = 0;
    int turn2 = 0;
    
    auto incrementStep1 = [&](int& value, int& temp) {
        if (turn1 % 2 == 0) {
            temp = value;  // Читаємо поточне значення
        } else {
            value = temp + 1;  // Інкрементуємо
        }
        ++turn1;
    };
    
    auto incrementStep2 = [&](int& value, int& temp) {
        if (turn2 % 2 == 0) {
            temp = value;  // Читаємо поточне значення
        } else {
            value = temp + 1;  // Інкрементуємо
        }
        ++turn2;
    };
    
    auto threadFunction = [&](int threadId) {
        int nonAtomicShared = 0; // Неатомарна версія змінної для простіших операцій
        
        for (int i = 0; i < 2 * SYNC_ITERATIONS; ++i) {
            std::unique_lock<std::mutex> lock(mtx);
            
            if (threadId == 1) {
                // Перший потік чекає своєї черги
                cv.wait(lock, [&] { return threadTurn; });
                incrementStep1(nonAtomicShared, tempValue1);
                sharedVariable.store(nonAtomicShared, std::memory_order_release);
                threadTurn = false;
                std::cout << "Thread 1: " << sharedVariable << std::endl;
                // threadTurn = true;
            } else {
                // Другий потік чекає своєї черги
                cv.wait(lock, [&] { return !threadTurn; });
                incrementStep2(nonAtomicShared, tempValue2);
                sharedVariable.store(nonAtomicShared, std::memory_order_release);
                threadTurn = true;
                std::cout << "Thread 2: " << sharedVariable << std::endl;
            }
            
            cv.notify_all();
        }
    };
    
    auto start = std::chrono::high_resolution_clock::now();
    std::thread t1(threadFunction, 1);
    std::thread t2(threadFunction, 2);
    t1.join();
    t2.join();
    
    double elapsed = measureTime(start);
    std::cout << "Синхронне паралельне додавання:" << std::endl;
    std::cout << "Очікуване значення: " << SYNC_ITERATIONS << std::endl;
    std::cout << "Отримане значення: " << sharedVariable << std::endl;
    std::cout << "Час виконання: " << elapsed << " секунд" << std::endl;
    std::cout << std::endl;
}
int main()
{
    synchronizedThreads();
    return 0;
}