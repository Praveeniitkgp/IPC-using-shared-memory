#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#define MAX_FOLLOWERS 100
#define HASH_SIZE 1000
#define DEFAULT_N 10

// Hash table structure for sum tracking
struct HashNode {
    int sum;
    bool used;
};

int main(int argc, char *argv[]) {
    int n = DEFAULT_N;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n > MAX_FOLLOWERS) {
            printf("Error: Maximum number of followers is %d\n", MAX_FOLLOWERS);
            exit(1);
        }
    }

    // Create shared memory
    key_t key = ftok("/", 65);
    int shmid = shmget(key, (4 + n) * sizeof(int), IPC_CREAT | IPC_EXCL | 0666);
    if (shmid == -1) {
        printf("Error: Another instance of leader is already running\n");
        exit(1);
    }

    // Attach shared memory
    int *M = (int *)shmat(shmid, NULL, 0);
    if (M == (int *)-1) {
        printf("Error attaching shared memory\n");
        exit(1);
    }

    // Initialize shared memory
    M[0] = n;          // Number of followers
    M[1] = 0;          // Number of followers joined
    M[2] = 0;          // Turn (0 for leader)

    printf("Wait for the moment.\n");

    // Wait for all followers to join
    while (M[1] < n) {
        usleep(100000);  // Sleep to reduce CPU usage
    }

    // Initialize hash table for sum tracking
    struct HashNode *hash_table = calloc(HASH_SIZE, sizeof(struct HashNode));
    
    // Random seed
    srand(time(NULL));
    
    bool sum_repeated = false;
    while (!sum_repeated) {
        // Leader's turn
        while (M[2] != 0 && M[2] != -1) {
            usleep(1000);  // Reduce CPU usage during busy wait
        }
        
        if (M[2] == -1) break;  // Termination signal
        
        // Generate random number and write to M[3]
        M[3] = rand() % 99 + 1;
        
        // Wait for all followers to complete their turns
        M[2] = 1;  // Give turn to first follower
        
        while (M[2] != 0) {
            usleep(1000);
        }
        
        // Calculate sum
        int sum = 0;
        for (int i = 3; i <= n + 3; i++) {
            sum += M[i];
        }
        
        // Print the current sum
        printf("%d", M[3]);
        for (int i = 4; i <= n + 3; i++) {
            printf(" + %d", M[i]);
        }
        printf(" = %d\n", sum);
        
        // Check if sum exists in hash table
        int hash_index = sum % HASH_SIZE;
        while (hash_table[hash_index].used) {
            if (hash_table[hash_index].sum == sum) {
                sum_repeated = true;
                M[2] = -1;  // Signal termination
                break;
            }
            hash_index = (hash_index + 1) % HASH_SIZE;
        }
        
        if (!sum_repeated) {
            hash_table[hash_index].sum = sum;
            hash_table[hash_index].used = true;
        }
    }
    
    // Wait for last follower to set M[2] = 0
    while (M[2] != 0) {
        usleep(1000);
    }
    
    // Cleanup
    free(hash_table);
    shmdt(M);
    shmctl(shmid, IPC_RMID, NULL);
    
    return 0;
}