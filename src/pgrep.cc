/**
 * @file
 * Parallel grep (project to IPS subject on FIT BUT)
 *
 * @author Michal ŠMAHEL (xsmahe01)
 */

#include <unistd.h>
#include <thread>
#include <queue>
#include <mutex>
#include <vector>
#include <iostream>
#include <regex>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/**
 * @def Number of fixed locks (locks required at every run independently to number of regexes)
 */
#define FIXED_LOCK_NUM 2

/**
 * @var Locks shared to all of the threads using a global variable
 *
 * @details
 * [0] - counting number of processed regexes critical section,
 * [1] - updating score critical section,
 * [FIXED_LOCK_NUM - 1], ... - variable number of locks for synchronizing start of processing a new line
 */
std::vector<std::mutex *> locks;

/**
 * @var Currently processed line
 */
char *processed_line;

/**
 * @var Total score (from all regexes) for currently processed line
 */
int total_score = 0;

/**
 * @var Number of regexes have already been processed by workers
 */
int processed_regexes = 0;

/**
 * @var Is input processing finished?
 */
bool finished = false;

/**
 * Reads new line from standard input
 *
 * @return Was it successful (EOL --> unsuccessful read)?
 */
bool read_line() {
    std::string line;
    char *str;
    if (std::getline(std::cin, line)) {
        str = (char *) malloc(sizeof(char) * (line.length() + 1));
        strcpy(str, line.c_str());
        processed_line = str;

        return true;
    } else {
        return false;
    }
}

/**
 * Thread worker function
 *
 * New thread workers starts here (it's something like their main())
 *
 * @param lock_i Index of the lock for waiting for new line in the locks array
 * @param score Score to add to the total score when the regex_string matches
 * @param regex_string Regex to check on currently processed line
 */
void worker(int lock_i, int score, const char *regex_string) {
    // Precompile regex
    std::regex regex(regex_string);

    while (!finished) {
        // Wait for input to be ready
        while (!locks[lock_i]->try_lock()) {
            usleep(1);
        }

        // We could end now, because everything is done
        if (finished) {
            return;
        }

        // START updating score critical section
        while (!locks[1]->try_lock()) {
            usleep(1);
        }

        if (std::regex_match(processed_line, regex)) {
            total_score += score;
        }
        locks[1]->unlock();
        // END updating score critical section

        // START counting processed regexes critical section
        // Notify that this worker is done with current line
        while (!locks[0]->try_lock()) {
            usleep(1);
        }

        processed_regexes++;
        locks[0]->unlock();
        // END counting processed regexes critical section
    }
}

int main(int argc, char **argv) {
    // We need an odd number (without implicit one) of input arguments
    // with minimal count of 3 (4 with path to the program)
    if ((argc - 1) % 2 != 1 || argc < 4) {
        fprintf(stderr, "You need to specify odd number of arguments with minimal count of 3");
        return 1;
    }

    // Get configurations
    // The first 2 arguments are path to the program and minimal score for writing a row
    int regex_num = (argc - 2) / 2;
    int lock_num = FIXED_LOCK_NUM + regex_num;
    int min_score = (int) strtol(argv[1], NULL, 10);

    // Create locks
    locks.resize(lock_num);
    for (int i = 0; i < lock_num; i++) {
        auto *new_lock = new std::mutex();
        locks[i] = new_lock;

        // All dynamic locks are locked by default
        // Workers must start waiting for input get ready after they are run
        if (i > FIXED_LOCK_NUM - 1) {
            locks[i]->lock();
        }
    }

    // Create threads
    std::vector < std::thread * > threads;
    threads.resize(regex_num);

    for (int i = 0; i < regex_num; i++) {
        // (0 - path to the program, 1 - minimal score, 2 + 2n - regexes, 3 + 2n - scores)
        char *regex = argv[2 * i + 2]; // [2], [4], [6], ...
        int score = (int) strtol(argv[2 * i + 3], NULL, 10); // [3], [5], [7], ...

        auto *new_thread = new std::thread(worker, FIXED_LOCK_NUM + i, score, regex);
        threads[i] = new_thread;
    }

    // Score counting
    while (read_line()) {
        // Input is ready for processing --> unlock one iteration of worker's job,
        // so one line will be processed
        for (int i = FIXED_LOCK_NUM; i < lock_num; i++) {
            locks[i]->unlock();
        }

        // Wait for all workers to be done with current line
        while (processed_regexes != regex_num) {
            usleep(1);
        }

        if (total_score >= min_score) {
            printf("%s\n", processed_line);
        }

        // Get ready for new line
        free(processed_line);
        processed_regexes = 0;
        total_score = 0;
    }

    // We're done, so unlock threads and tell them about this (they could end now)
    for (int i = FIXED_LOCK_NUM; i < lock_num; i++) {
        locks[i]->unlock();
    }
    finished = true;

    // Cleaning stuff
    // Deallocate threads
    for (int i = 0; i < regex_num; i++) {
        threads[i]->join();
        delete threads[i];
    }

    // Deallocate locks
    for (int i = 0; i < lock_num; i++) {
        delete locks[i];
    }
}
