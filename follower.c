#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

#define DEFAULT_NF 1

void follower_process(int follower_num, int *M) {
    printf("follower %d joins\n", follower_num);
    
 
    srand(time(NULL) ^ (follower_num * 1000));
    
    while (1) {
        
        while (M[2] != follower_num && M[2] != -follower_num) {
            usleep(1000);
        }
        
        if (M[2] == -follower_num) {
            // Termination signal received
            printf("follower %d leaves\n", follower_num);
            
            // Set turn for next follower or back to leader
            if (follower_num == M[0]) {
                M[2] = 0;  // Back to leader
            } else {
                M[2] = -(follower_num + 1);  
            }
            break;
        }
        
        // Generate random number and write to shared memory
        M[follower_num + 3] = rand() % 9 + 1;
        
        // Set turn for next follower or back to leader
        if (follower_num == M[0]) {
            M[2] = 0;  // Back to leader
        } else {
            M[2] = follower_num + 1;  // Next follower's turn
        }
    }
    
    exit(0);
}

int main(int argc, char *argv[]) {
    int nf = DEFAULT_NF;
    if (argc > 1) {
        nf = atoi(argv[1]);
    }
    
    // Get shared memory
    key_t key = ftok("/", 65);
    int shmid = shmget(key, 0, 0666);
    if (shmid == -1) {
        printf("Error: Leader is not running\n");
        exit(1);
    }
    
    // Attach shared memory
    int *M = (int *)shmat(shmid, NULL, 0);
    if (M == (int *)-1) {
        printf("Error attaching shared memory\n");
        exit(1);
    }
    
    pid_t *children = malloc(nf * sizeof(pid_t));
    
    // Create follower processes
    for (int i = 0; i < nf; i++) {
        // Atomically increment M[1] and get follower number
        int follower_num = ++M[1];
        
        // Check if maximum followers reached
        if (follower_num > M[0]) {
            printf("follower error: %d followers have already joined\n", M[0]);
            continue;
        }
        
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            follower_process(follower_num, M);
        } else if (pid > 0) {
            // Parent process
            children[i] = pid;
        } else {
            printf("Error creating follower process\n");
            exit(1);
        }
    }
    
    // Wait for all children to complete
    for (int i = 0; i < nf; i++) {
        waitpid(children[i], NULL, 0);
    }
    
    // Cleanup
    free(children);
    shmdt(M);
    
    return 0;
}