import matplotlib.pyplot as plt
import numpy as np

# Дані з виводу програми
matrix_sizes = [2, 5, 10, 50, 100, 200, 500]
threads = list(range(1, 17))

# Час виконання для різних розмірів матриць
times = {
    2: [0.43, 0.62, 0.89, 0.83, 1.01, 0.96, 0.88, 0.91, 1.00, 0.95, 0.98, 1.46, 0.85, 0.87, 0.66, 0.54],
    5: [0.20, 0.36, 0.75, 0.98, 1.19, 1.37, 1.57, 1.94, 2.08, 2.27, 2.83, 2.76, 2.01, 3.25, 3.19, 3.81],
    10: [0.25, 0.53, 0.78, 1.05, 1.16, 1.32, 1.64, 1.87, 1.85, 2.31, 2.63, 2.93, 3.67, 5.36, 4.99, 4.17],
    50: [1.46, 1.26, 1.38, 1.58, 1.51, 1.73, 1.51, 2.13, 2.27, 2.78, 2.97, 3.09, 3.46, 3.38, 3.36, 3.69],
    100: [7.97, 8.11, 6.81, 4.89, 6.00, 6.45, 5.46, 3.85, 6.27, 5.66, 4.14, 4.34, 4.76, 4.84, 4.64, 5.72],
    200: [67.29, 46.93, 35.04, 31.15, 20.24, 21.52, 22.89, 16.65, 18.23, 15.42, 15.37, 14.29, 14.38, 14.85, 13.76, 17.69],
    500: [1018.27, 512.43, 376.68, 276.07, 234.50, 200.65, 189.74, 199.18, 204.92, 198.58, 179.55, 171.28, 161.84, 150.85, 148.88, 147.35]
}

# Прискорення відносно 1 потоку
speedups = {
    2: [1.00, 0.69, 0.48, 0.52, 0.42, 0.44, 0.49, 0.47, 0.43, 0.45, 0.44, 0.29, 0.50, 0.49, 0.64, 0.79],
    5: [1.00, 0.54, 0.26, 0.20, 0.17, 0.14, 0.13, 0.10, 0.10, 0.09, 0.07, 0.07, 0.10, 0.06, 0.06, 0.05],
    10: [1.00, 0.48, 0.32, 0.24, 0.22, 0.19, 0.15, 0.14, 0.14, 0.11, 0.10, 0.09, 0.07, 0.05, 0.05, 0.06],
    50: [1.00, 1.16, 1.06, 0.93, 0.97, 0.85, 0.97, 0.69, 0.64, 0.53, 0.49, 0.47, 0.42, 0.43, 0.44, 0.40],
    100: [1.00, 0.98, 1.17, 1.63, 1.33, 1.24, 1.46, 2.07, 1.27, 1.41, 1.93, 1.84, 1.67, 1.65, 1.72, 1.39],
    200: [1.00, 1.43, 1.92, 2.16, 3.32, 3.13, 2.94, 4.04, 3.69, 4.36, 4.38, 4.71, 4.68, 4.53, 4.89, 3.80],
    500: [1.00, 1.99, 2.70, 3.69, 4.34, 5.07, 5.37, 5.11, 4.97, 5.13, 5.67, 5.95, 6.29, 6.75, 6.84, 6.91]
}

# Створюємо графіки
plt.figure(figsize=(14, 8))
for size in matrix_sizes:
    plt.plot(threads, times[size], marker='o', label=f'Розмір {size}x{size}')
plt.title('Час виконання множення матриць залежно від кількості потоків')
plt.xlabel('Кількість потоків')
plt.ylabel('Час (мс)')
plt.grid(True)
plt.legend()
plt.tight_layout()

# Графік прискорення
plt.figure(figsize=(14, 8))
for size in matrix_sizes:
    plt.plot(threads, speedups[size], marker='o', label=f'Розмір {size}x{size}')
plt.axhline(y=1, color='r', linestyle='--', label='Лінія однакової продуктивності')
plt.title('Прискорення відносно 1 потоку залежно від кількості потоків')
plt.xlabel('Кількість потоків')
plt.ylabel('Прискорення (разів)')
plt.grid(True)
plt.legend()
plt.tight_layout()

# Знаходження оптимальної кількості потоків для кожного розміру матриці
optimal_threads = {}
for size in matrix_sizes:
    optimal_threads[size] = threads[np.argmin(times[size])]

print("Оптимальна кількість потоків для кожного розміру матриці:")
for size, optimal in optimal_threads.items():
    print(f"Розмір {size}x{size}: {optimal} потоків")

# Графік залежності оптимального числа потоків від розміру матриць
plt.figure(figsize=(12, 7))
plt.plot(matrix_sizes, [optimal_threads[size] for size in matrix_sizes], marker='o', linewidth=2)
plt.title('Оптимальна кількість потоків залежно від розміру матриці')
plt.xlabel('Розмір матриці')
plt.ylabel('Оптимальна кількість потоків')
plt.grid(True)
plt.tight_layout()

# Аналіз ефективності паралелізації
plt.figure(figsize=(14, 8))
for size in [100, 200, 500]:  # Використовуємо лише більші матриці, де є реальне прискорення
    efficiency = [speedups[size][i-1]/i for i in threads]
    plt.plot(threads, efficiency, marker='o', label=f'Розмір {size}x{size}')
plt.axhline(y=1, color='r', linestyle='--', label='Ідеальна ефективність')
plt.title('Ефективність паралелізації залежно від кількості потоків')
plt.xlabel('Кількість потоків')
plt.ylabel('Ефективність (прискорення/потоки)')
plt.grid(True)
plt.legend()
plt.tight_layout()

# Аналіз часу виконання залежно від розміру матриці
plt.figure(figsize=(12, 7))
sizes_squared = [size**3 for size in matrix_sizes]  # O(n^3) для множення матриць
single_thread_times = [times[size][0] for size in matrix_sizes]
plt.scatter(sizes_squared, single_thread_times)
plt.title('Залежність часу виконання від розміру матриці (1 потік)')
plt.xlabel('Розмір матриці в кубі (n^3)')
plt.ylabel('Час виконання (мс)')
plt.grid(True)
plt.tight_layout()

# Визначення для яких розмірів ефект більше
best_speedup = {}
for size in matrix_sizes:
    best_speedup[size] = max(speedups[size])

print("\nМаксимальне прискорення для кожного розміру матриці:")
for size, max_speedup in best_speedup.items():
    print(f"Розмір {size}x{size}: {max_speedup:.2f}x")

print("\nЧас виконання для матриці 500x500:")
for i, t in enumerate(times[500], 1):
    print(f"{i} потоків: {t:.2f} мс")

# Знаходимо, при якій кількості потоків досягається найкраще прискорення для кожного розміру
best_speedup_threads = {}
for size in matrix_sizes:
    best_speedup_threads[size] = threads[np.argmax(speedups[size])]

print("\nКількість потоків для найкращого прискорення:")
for size, optimal in best_speedup_threads.items():
    print(f"Розмір {size}x{size}: {optimal} потоків")

# Логарифмічний масштаб для кращого відображення невеликих матриць
plt.figure(figsize=(14, 8))
for size in matrix_sizes:
    plt.plot(threads, times[size], marker='o', label=f'Розмір {size}x{size}')
plt.title('Час виконання множення матриць (логарифмічна шкала)')
plt.xlabel('Кількість потоків')
plt.ylabel('Час (мс)')
plt.yscale('log')
plt.grid(True)
plt.legend()
plt.tight_layout()

plt.show()