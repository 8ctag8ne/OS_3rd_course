#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <vector>
#include <condition_variable>

// Константи для налаштування експерименту
constexpr long long ITERATIONS = 1000000000; // 10^9 ітерацій
constexpr int NUM_THREADS = 2;

// Функція для вимірювання часу виконання
double measureTime(std::chrono::time_point<std::chrono::high_resolution_clock> start) {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    return duration / 1000.0; // повертаємо час у секундах
}

// === 1. Без синхронізації ===
void withoutSync() {
    long long sharedVariable = 0;
    
    auto incrementFunction = [&sharedVariable]() {
        for (long long i = 0; i < ITERATIONS; ++i) {
            sharedVariable++; // Небезпечне збільшення без синхронізації
        }
    };
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::thread t1(incrementFunction);
    std::thread t2(incrementFunction);
    
    t1.join();
    t2.join();
    
    double elapsed = measureTime(start);
    
    std::cout << "Без синхронізації:" << std::endl;
    std::cout << "Очікуване значення: " << 2 * ITERATIONS << std::endl;
    std::cout << "Отримане значення:  " << sharedVariable << std::endl;
    std::cout << "Різниця:            " << (2 * ITERATIONS - sharedVariable) << std::endl;
    std::cout << "Час виконання:      " << elapsed << " секунд" << std::endl;
    std::cout << std::endl;
}

// === 2. З використанням mutex ===
void withMutex() {
    long long sharedVariable = 0;
    std::mutex mtx;
    
    auto incrementFunction = [&sharedVariable, &mtx]() {
        for (long long i = 0; i < ITERATIONS; ++i) {
            std::lock_guard<std::mutex> lock(mtx);
            sharedVariable++; // Безпечне збільшення з блокуванням mutex
        }
    };
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::thread t1(incrementFunction);
    std::thread t2(incrementFunction);
    
    t1.join();
    t2.join();
    
    double elapsed = measureTime(start);
    
    std::cout << "З використанням mutex:" << std::endl;
    std::cout << "Очікуване значення: " << 2 * ITERATIONS << std::endl;
    std::cout << "Отримане значення:  " << sharedVariable << std::endl;
    std::cout << "Різниця:            " << (2 * ITERATIONS - sharedVariable) << std::endl;
    std::cout << "Час виконання:      " << elapsed << " секунд" << std::endl;
    std::cout << std::endl;
}

// === 3. З використанням atomic ===
void withAtomic() {
    std::atomic<long long> sharedVariable(0);
    
    auto incrementFunction = [&sharedVariable]() {
        for (long long i = 0; i < ITERATIONS; ++i) {
            sharedVariable++; // Атомарне збільшення
        }
    };
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::thread t1(incrementFunction);
    std::thread t2(incrementFunction);
    
    t1.join();
    t2.join();
    
    double elapsed = measureTime(start);
    
    std::cout << "З використанням atomic:" << std::endl;
    std::cout << "Очікуване значення: " << 2 * ITERATIONS << std::endl;
    std::cout << "Отримане значення:  " << sharedVariable << std::endl;
    std::cout << "Різниця:            " << (2 * ITERATIONS - sharedVariable) << std::endl;
    std::cout << "Час виконання:      " << elapsed << " секунд" << std::endl;
    std::cout << std::endl;
}

// === 4. Оптимізована версія з використанням atomic і часткового накопичення результату ===
void optimizedAtomic() {
    std::atomic<long long> sharedVariable(0);
    
    auto incrementFunction = [&sharedVariable]() {
        // Використовуємо локальний лічильник для зменшення кількості атомарних операцій
        long long localCounter = 0;
        const int batchSize = 10000;
        
        for (long long i = 0; i < ITERATIONS; ++i) {
            localCounter++;
            
            // Оновлюємо атомарну змінну тільки кожні batchSize ітерацій
            if (localCounter % batchSize == 0) {
                sharedVariable.fetch_add(batchSize, std::memory_order_relaxed);
            }
        }
        
        // Додаємо залишок
        int remainder = localCounter % batchSize;
        if (remainder > 0) {
            sharedVariable.fetch_add(remainder, std::memory_order_relaxed);
        }
    };
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::thread t1(incrementFunction);
    std::thread t2(incrementFunction);
    
    t1.join();
    t2.join();
    
    double elapsed = measureTime(start);
    
    std::cout << "Оптимізований atomic:" << std::endl;
    std::cout << "Очікуване значення: " << 2 * ITERATIONS << std::endl;
    std::cout << "Отримане значення:  " << sharedVariable << std::endl;
    std::cout << "Різниця:            " << (2 * ITERATIONS - sharedVariable) << std::endl;
    std::cout << "Час виконання:      " << elapsed << " секунд" << std::endl;
    std::cout << std::endl;
}


int main() {
    std::cout << "Демонстрація роботи паралельних потоків зі спільною змінною" << std::endl;
    std::cout << "Кількість ітерацій: " << ITERATIONS << std::endl;
    std::cout << "Кількість потоків: " << NUM_THREADS << std::endl;
    std::cout << "=====================================================\n" << std::endl;
    
    // Виконуємо експерименти
    withoutSync();
    withMutex();
    withAtomic();
    optimizedAtomic();
    
    
    return 0;
}