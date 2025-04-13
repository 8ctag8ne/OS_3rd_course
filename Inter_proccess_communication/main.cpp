#include <iostream>
#include <thread>
#include <mutex>
#include <optional>
#include <condition_variable>
#include <chrono>
#include <functional>

int f(int x) {
    if (x % 48 == 0) std::this_thread::sleep_for(std::chrono::seconds(x / 2));
    return x % 2;
}

int g(int x) {
    if (x % 31 == 0) std::this_thread::sleep_for(std::chrono::seconds(x / 31 * 10));
    return (x % 3) ? 1 : 0;
}

std::optional<int> f_result, g_result;
std::mutex mtx;
std::condition_variable cv;

void compute(std::function<int(int)> func, int x, std::optional<int>& result) {
    int r = func(x);
    {
        std::lock_guard<std::mutex> lock(mtx);
        result = r;
    }
    cv.notify_all(); // повідомляємо про оновлення
}

int main() {
    int x;
    std::cout<<"Enter the x value for computing f(x) && g(x): ";
    std::cin >> x;

    std::thread tf(compute, f, x, std::ref(f_result));
    std::thread tg(compute, g, x, std::ref(g_result));

    bool ask_user = true;

    std::unique_lock<std::mutex> lock(mtx);
    while (true) 
    {
        if (!cv.wait_for(lock, std::chrono::seconds(10), [] 
        {
            return (f_result && *f_result == 0) ||
                   (g_result && *g_result == 0) ||
                   (f_result && g_result);
        })) 

        {
            // Якщо чекали 10 сек і нічого
            if (ask_user) 
            {
                std::cout << "Still computing... Options:\n1) Continue\n2) Stop\n3) Continue without asking\n> ";
                int choice;
                std::cin >> choice;
                if (choice == 2) {
                    std::cout << "Stopped by user.\n";
                    break;
                } else if (choice == 3) {
                    ask_user = false;
                }
            }
            continue;
        }

        // Результати вже готові, розбираємо
        if (f_result && *f_result == 0) 
        {
            std::cout << "f(x) = 0\nResult: false\n";
            break;
        }
        if (g_result && *g_result == 0) 
        {
            std::cout << "g(x) = 0\nResult: false\n";
            break;
        }
        if (f_result && g_result) 
        {
            std::cout << "f(x) = " << *f_result << "\ng(x) = " << *g_result << "\nResult: true\n";
            break;
        }
    }

    tf.detach();
    tg.detach();
    return 0;
}
