#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
// Define CLOCK_MONOTONIC for Windows
#define CLOCK_MONOTONIC 1

// Implement clock_gettime for Windows
int clock_gettime(int ignored, struct timespec *tp) {
    LARGE_INTEGER frequency, counter;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);

    tp->tv_sec = counter.QuadPart / frequency.QuadPart;
    tp->tv_nsec = (counter.QuadPart % frequency.QuadPart) * 1000000000 / frequency.QuadPart;
    return 0;
}
#endif

const int STATUS_OK = 0;
const int STATUS_FAILED = 1;
const int STATUS_STOPPED = 2;

const int MAX_NUMBER_OF_SAMPLES = 31;

int readSensor(int *value) {
    static int v = 0;
    *value = v;
    v++;
    return (v > MAX_NUMBER_OF_SAMPLES) ? STATUS_STOPPED : STATUS_OK;
}

char **messages = NULL;

static const int VALUE_LOW_MSG = 0;
static const int VALUE_HIGH_MSG = 1;
static const int ERROR_MSG = 2;

void initialize() {
    if (messages == NULL) {
        messages = (char **)malloc(sizeof(char *) * 3);
        if (messages == NULL) {
            printf("Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }

        messages[0] = (char *)malloc(128);
        messages[1] = (char *)malloc(128);
        messages[2] = (char *)malloc(128);

        if (messages[0] == NULL || messages[1] == NULL || messages[2] == NULL) {
            printf("Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }

        strcpy(messages[VALUE_LOW_MSG], "Low");
        strcpy(messages[VALUE_HIGH_MSG], "High");
        strcpy(messages[ERROR_MSG], "Error occurred");
    }
}

void finalize() {
    if (messages) {
        free(messages[0]);
        free(messages[1]);
        free(messages[2]);
        free(messages);
        messages = NULL;  // Prevent double free
    }
}

void printMessage(int msgIndex, int value) {
    if (msgIndex < 0 || msgIndex > 2) {
        printf("Value: %d, State: Undefined\n", value);
    } else {
        printf("Value: %d, State: %s\n", value, messages[msgIndex]);
    }
    fflush(stdout);
}

void reportSensorFailure() {
    finalize();
    initialize();
    printMessage(ERROR_MSG, 0);
}

void handleSensorValue(int value) {
    int index = -1;
    initialize();
    if (value >= 0 && value <= 10) {
        index = VALUE_LOW_MSG;
    } else if (value > 10 && value <= 20) {
        index = VALUE_HIGH_MSG;
    }
    printMessage(index, value);
}

void checkTimingFailure(int max_time_us) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    int sensorValue;
    int status = STATUS_OK;
    int count = 0;
    while (count < MAX_NUMBER_OF_SAMPLES) {
        status = readSensor(&sensorValue);
        if (status == STATUS_STOPPED) {
            break;
        } else if (status == STATUS_FAILED) {
            reportSensorFailure();
            return;
        }
        handleSensorValue(sensorValue);
        count++;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    long elapsed_time_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;

    // Print timing result
    printf("\n--------------------------------------------------\n");
    if (elapsed_time_us > max_time_us) {
        printf("🚨 Timing failure: Execution took %ld µs (exceeded %d µs)\n", elapsed_time_us, max_time_us);
    } else {
        printf("✅ Timing success: Execution took %ld µs (within %d µs)\n", elapsed_time_us, max_time_us);
    }
    printf("--------------------------------------------------\n");
}

int main() {
    char choice;

    do {
        int max_time_us;

        // **User message asking for expected run time in microseconds**
        printf("\n==================================================\n");
        printf("🕒 Please enter the expected run time in microseconds (µs): ");

        // Ensure the input is a valid integer
        while (scanf("%d", &max_time_us) != 1) {
            printf("❌ Invalid input! Please enter a valid number: ");
            while (getchar() != '\n'); // Clear input buffer
        }

        checkTimingFailure(max_time_us);
        finalize();

        // **Ask user if they want to run again**
        printf("\nWould you like another run? (y to continue, q to quit): ");

        // Clear input buffer and get user choice
        while (getchar() != '\n');  // Clear previous input
        choice = getchar();

    } while (choice == 'y' || choice == 'Y');

    printf("\n🚪 Exiting program. Goodbye!\n");
    return 0;
}
