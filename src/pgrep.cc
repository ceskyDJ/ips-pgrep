#include <cstdio>
#include <unistd.h>
#include <thread>
#include <queue>
#include <mutex>
#include <vector>
#include <iostream>
#include <cstring>
#include <cstdlib>

/**
 * @def Total number of locks used in the program
 */
#define LOCK_NUM 15

/**
 * @var Locks shared to all of the threads using a global variable
 */
std::vector<std::mutex *> locks;

/**
 * @var Currently processed line
 */
char *processed_line;

char *read_line(int *res) {
    std::string line;
    char *str;
    if (std::getline(std::cin, line)) {
        str = (char *) malloc(sizeof(char) * (line.length() + 1));
        strcpy(str, line.c_str());
        *res = 1;

        return str;
    } else {
        *res = 0;

        return nullptr;
    }
}

void worker(int id) {
    printf("Worker #%i started\n", id);
}

int main(int argc, char **argv) {
    // We need an odd number (without implicit one) of input arguments
    // with minimal count of 3 (4 with path to the program)
    if ((argc - 1) % 2 != 1 || argc < 4) {
        std::cerr << "You need to specify odd number of arguments with minimal count of 3";
        return 1;
    }

    // Get configurations
    // The first 2 arguments are path to the program and minimal score for writing a row
    int regex_num = argc - 2;
    int min_score = (int) strtol(argv[1], nullptr, 10);

    // Create locks
    locks.resize(LOCK_NUM);
    for (int i = 0; i < LOCK_NUM; i++) {
        auto *new_lock = new std::mutex();
        locks[i] = new_lock;

        // Lock could be locked now, if needed
//        (*(locks[i])).lock();
    }

    // Create threads
    std::vector < std::thread * > threads;
    threads.resize(regex_num);

    for (int i = 0; i < regex_num; i++) {
        auto *new_thread = new std::thread(worker, i);
        threads[i] = new_thread;
    }

    // Score counting
    int res;
    processed_line = read_line(&res);
    while (res) {
        printf("%s\n", processed_line);
        free(processed_line);
        processed_line = read_line(&res);
    }

    // Cleaning stuff
    // Deallocate threads
    for (int i = 0; i < regex_num; i++) {
        (*(threads[i])).join();
        delete threads[i];
    }

    // Deallocate locks
    for (int i = 0; i < LOCK_NUM; i++) {
        delete locks[i];
    }
}
