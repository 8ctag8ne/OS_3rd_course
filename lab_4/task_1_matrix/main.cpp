#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <random>
#include <iomanip>
#include <algorithm>


// Мьютекс для синхронізації виводу
std::mutex output_mutex;

// Структура для зберігання матриць
struct Matrix {
    int rows;
    int cols;
    std::vector<std::vector<int>> data;

    Matrix(int r, int c) : rows(r), cols(c) {
        data.resize(rows, std::vector<int>(cols, 0));
    }

    // Заповнення матриці випадковими значеннями
    void fillRandom() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1, 10);
        
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                data[i][j] = dis(gen);
            }
        }
    }

    // Виведення матриці
    void print() const {
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                std::cout << std::setw(5) << data[i][j];
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
};

// Глобальна змінна для контролю виводу результатів
bool showElementCalculation = true;

// Функція, яка обчислює елемент результуючої матриці
void calculateElement(const Matrix& A, const Matrix& B, Matrix& C, int row, int col) {
    C.data[row][col] = 0;
    for (int k = 0; k < A.cols; k++) {
        C.data[row][col] += A.data[row][k] * B.data[k][col];
    }
    
    // Захоплюємо мьютекс для безпечного виводу результату (якщо вивід увімкнено)
    if (showElementCalculation) {
        std::lock_guard<std::mutex> lock(output_mutex);
        std::cout << "Потік обчислив: [" << row << "," << col << "] = " << C.data[row][col] << "\n";
        std::cout.flush();
    }
}

// Функція для множення матриць з використанням заданої кількості потоків
Matrix multiplyMatrices(const Matrix& A, const Matrix& B, int numThreads) {
    if (A.cols != B.rows) {
        throw std::invalid_argument("Несумісні розміри матриць для множення!");
    }

    Matrix C(A.rows, B.cols);
    std::vector<std::thread> threads;
    
    // Обмежуємо кількість потоків до кількості елементів результуючої матриці
    int totalElements = A.rows * B.cols;
    if (numThreads > totalElements) {
        numThreads = totalElements;
    }

    // Розрахунок, скільки елементів буде обчислювати кожен потік
    int elementsPerThread = totalElements / numThreads;
    int extraElements = totalElements % numThreads;
    
    int currentElement = 0;

    // Створюємо та запускаємо потоки
    for (int t = 0; t < numThreads; t++) {
        int elementsForThisThread = elementsPerThread + (t < extraElements ? 1 : 0);
        int startElement = currentElement;
        int endElement = startElement + elementsForThisThread - 1;
        
        // Перетворюємо індекси елементів на рядки та стовпці
        int startRow = startElement / B.cols;
        int startCol = startElement % B.cols;
        int endRow = endElement / B.cols;
        int endCol = endElement % B.cols;
        
        // Додаємо потік для обчислення групи елементів
        threads.push_back(std::thread([&A, &B, &C, startRow, startCol, endRow, endCol]() {
            for (int i = startRow; i <= endRow; i++) {
                for (int j = (i == startRow ? startCol : 0); j <= (i == endRow ? endCol : B.cols - 1); j++) {
                    calculateElement(A, B, C, i, j);
                }
            }
        }));
        
        currentElement += elementsForThisThread;
    }

    // Чекаємо на завершення всіх потоків
    for (auto& thread : threads) {
        thread.join();
    }

    return C;
}

// Вимірювання часу виконання множення для різної кількості потоків
void benchmarkMultiplication(const Matrix& A, const Matrix& B, int maxThreads) {
    std::cout << "Benchmark результати:" << std::endl;
    std::cout << "Потоків | Час виконання (мс) | Прискорення відносно 1 потоку" << std::endl;
    std::cout << "-----------------------------------------------------------" << std::endl;
    
    // Змінна для зберігання часу виконання з 1 потоком (для розрахунку прискорення)
    double singleThreadTime = 0.0;
    
    // Визначаємо кроки для тестування потоків (для великих значень maxThreads)
    std::vector<int> threadCounts;
    
    // Тестуємо кожну можливу кількість потоків
    for (int t = 1; t <= maxThreads; t++) {
        threadCounts.push_back(t);
    }

    // Вимикаємо вивід для кожного елемента матриці під час бенчмарку
    bool originalOutputState = showElementCalculation;
    showElementCalculation = false;
    
    // Виконуємо тести для вибраних кількостей потоків
    for (int threads : threadCounts) {
        // Виконуємо кілька повторень для більш точних результатів
        const int repeats = 3;
        double totalTime = 0.0;
        
        for (int r = 0; r < repeats; r++) {
            auto start = std::chrono::high_resolution_clock::now();
            
            Matrix C = multiplyMatrices(A, B, threads);
            
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> duration = end - start;
            totalTime += duration.count();
        }
        
        // Розраховуємо середній час
        double avgTime = totalTime / repeats;
        
        // Зберігаємо час для 1 потоку
        if (threads == 1) {
            singleThreadTime = avgTime;
        }
        
        double speedup = singleThreadTime > 0 ? singleThreadTime / avgTime : 0.0;
        
        std::cout << std::setw(7) << threads << " | " 
                  << std::setw(18) << std::fixed << std::setprecision(2) << avgTime << " | "
                  << std::setw(15) << std::fixed << std::setprecision(2) << speedup << "x" << std::endl;
    }
    
    // Відновлюємо початковий стан виводу
    showElementCalculation = originalOutputState;
}

int main() {
    // Розміри матриць
    int n = 10;  // кількість рядків матриці A
    int m = 10;  // кількість стовпців матриці A / рядків матриці B
    int k = 10;  // кількість стовпців матриці B
    
    // Створення та заповнення матриць
    Matrix A(n, m);
    Matrix B(m, k);
    A.fillRandom();
    B.fillRandom();
    
    std::cout << "Матриця A:" << std::endl;
    A.print();
    
    std::cout << "Матриця B:" << std::endl;
    B.print();
    
    // Визначення кількості потоків
    int numThreads = 4;  // Можна змінити на потрібну кількість потоків
    
    std::cout << "Множення матриць з використанням " << numThreads << " потоків:" << std::endl;
    Matrix C = multiplyMatrices(A, B, numThreads);
    
    std::cout << "Результат множення (матриця C):" << std::endl;
    C.print();
    
    // Запуск бенчмарку для різної кількості потоків
    int maxThreads = std::thread::hardware_concurrency();
    std::cout << "Максимальна кількість апаратних потоків: " << maxThreads << std::endl;
    
    // Дослідження швидкодії для різних розмірностей матриць
    std::cout << "\nДослідження швидкодії для різних розмірностей матриць:" << std::endl;
    
    // Вектор розмірів матриць для тестування
    std::vector<int> matrixSizes = {2, 5, 10, 50, 100, 200, 500};
    // std::vector<int> matrixSizes = {100};
    
    for (int size : matrixSizes) {
        std::cout << "\nРозмір матриць: " << size << "x" << size << std::endl;
        
        // Створення більших матриць для тестування
        Matrix largeA(size, size);
        Matrix largeB(size, size);
        largeA.fillRandom();
        largeB.fillRandom();
        
        // Проведення бенчмарку
        benchmarkMultiplication(largeA, largeB, maxThreads);
    }
    
    return 0;
}