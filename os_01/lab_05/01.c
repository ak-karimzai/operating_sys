#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h> 
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>

const int N = 9;
const size_t shm_size = (9 + 2) * sizeof(int);

int* shm_at_add;
int* shm_buffer;
int* shm_pos_consum;
int* shm_pos_prod;

#define SB 0
#define SE 1
#define SM 2

#define P -1
#define V 1

struct sembuf prod_start[2] = { { SE, P, 0 }, 
                                { SB, P, 0 } };

struct sembuf prod_stop [2] = { { SB, V, 0 }, 
                                { SM, V, 0 } };

struct sembuf consum_start[2] = { { SM, P, 0 },
                                { SB, P, 0 } };

struct sembuf consum_stop [2] = { { SB, V, 0 }, 
                                { SE, V, 0 } };

void prod(const int semid, const int value, const int prod_id);
void consum(const int semid, const int value, const int consum_id);
void create_prod(const int id, const int semid, int value, int COUNT);
void create_consum(const int id, const int semid, int COUNT);

int main() {
    srand(0);
    int shmid;  // shared memory id
    int semid;  // semaphore id

    pid_t parent_pid = getpid();
    printf("Parent pid: %i\n", parent_pid);
    if ((shmid = shmget(IPC_PRIVATE, shm_size, IPC_CREAT | S_IRWXU | S_IRWXG | S_IRWXO)) == -1) {
        perror("Unable to create shared area"); exit(1);
    }

    shm_at_add = shmat(shmid, NULL, 0);

    if (*(char*)shm_at_add == -1) {
        perror("!!! can't attach memory"); exit(1);
    }

    shm_pos_prod = shm_at_add;
    shm_pos_consum = shm_at_add + sizeof(int);
    shm_buffer = shm_at_add + 2 * sizeof(int);
    (*shm_pos_prod) = 0;
    (*shm_pos_consum) = 0;
    
    if ((semid = semget(IPC_PRIVATE, 3, IPC_CREAT | S_IRWXU | S_IRWXG | S_IRWXO)) == -1) {
        perror("Unable to create semapthores"); exit(1);
    }

    int ctl_SB = semctl(semid, SB, SETVAL, 1);
    int ctl_SE  = semctl(semid, SE , SETVAL, N);
    int ctl_SM   = semctl(semid, SM  , SETVAL, 0);

    if (ctl_SB == -1 || ctl_SE == -1 || ctl_SM == -1) {
        perror("!!! Cannot set control semaphores"); exit(1); }
    
    create_consum(0, semid, 4);
    create_consum(1, semid, 5);

    create_prod(0, semid, 0, 3);
    create_prod(1, semid, 10, 3);
    create_prod(2, semid, 100, 3);

    int status;
    for (int i = 0; i < 5; i++) { wait(&status); }

    if (shmdt(shm_at_add) == -1) {
        perror("!!! Can't detach shared memory segment"); exit(1);
    }
    exit(0);
}

void prod(const int semid, const int value, const int prod_id) {
    sleep(rand() % 3);
    if (semop(semid, prod_start, 2) == -1) {
        perror("!!! prod can't make operation on semaphors. ");
        exit(1);
    }
    shm_buffer[*shm_pos_prod] = value;
    printf("prod  %d   pos %d ===> produced %d\n", prod_id, *shm_pos_prod, shm_buffer[*shm_pos_prod]);
    (*shm_pos_prod)++;
    if (semop(semid, prod_stop, 2) == -1) {
        perror("!!! prod can't make operation on semaphors. ");
        exit(1);
    }
}

void consum(const int semid, const int value, const int consum_id) {
    sleep(rand() % 3);
    if (semop(semid, consum_start, 2) == -1) {
        perror("!!! prod can't make operation on semaphors. ");
        exit(1);
    }
    printf("consum  %d   pos %d <=== consumed %d\n", consum_id, *shm_pos_consum, shm_buffer[*shm_pos_consum]);
    (*shm_pos_consum)++;

    if (semop(semid, consum_stop, 2) == -1) {
        perror("!!! prod can't make operation on semaphors. ");
        exit(1);
    }
}

void create_prod(const int id, const int semid, int value, int COUNT) {
    pid_t child1_pid;
    if ((child1_pid = fork()) == -1) {
        perror("!!! Can't fork"); exit(1);
    }
    if(child1_pid == 0) {
        // child;
        printf("Created prod %d\n", id);
        for (int i = 0; i < COUNT; i++) {
            prod(semid, value, id); value++;
        }

        printf("\tproducer  %d   finished. Terminating...\n", id);
        exit(0);
    }
}

void create_consum(const int id, const int semid, int COUNT) {
    pid_t child_pid;
    if ((child_pid = fork()) == -1) {
        perror("!!! Can't fork"); exit(1);
    }
    if(child_pid == 0) {
        printf("Created consumer %d\n", id);
        // child
        for (int i = 0; i < COUNT; i++) {
            consum(semid, i, id);
        }

        printf("\tconsumer  %d   finished. Terminating...\n", id);
        exit(0);
    }
}