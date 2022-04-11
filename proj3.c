#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "common_threads.h"
#include "common.h"

// d[4] = [W[L, S, R],
//         N[L, S, R],
//         E[L, S, R],
//         S[L, S, R]]
sem_t d[4][3];
sem_t lock;
sem_t newD;

char curr_d = ' ';
int curr_m[3] = {0, 0, 0};


int tLeft = 5;
int tStright = 4;
int tRight = 3;

// directions datatype
// note: directions will be labled as follows:
// W(<), E(>), S(v), and N(^)
typedef struct _directions {
char dir_original;
char dir_target;
} directions;

typedef struct _carThread{
    int cid;
    double arrivalTime;
    directions dir;
} carThread;


void carsInit(carThread c[]){
    for(int i = 0; i < 8; i++){
        c[i].cid = i+1;
        c[i].arrivalTime = c[i].cid * 1.1;
    }

    c[0].dir.dir_original = '^';
    c[0].dir.dir_target = '^';
    c[1].dir.dir_original = '^';
    c[1].dir.dir_target = '^';
    c[2].dir.dir_original = '^';
    c[2].dir.dir_target = '<';
    c[3].dir.dir_original = 'v';
    c[3].dir.dir_target = 'v';
    c[4].dir.dir_original = 'v';
    c[4].dir.dir_target = '>';
    c[5].dir.dir_original = '^';
    c[5].dir.dir_target = '^';
    c[6].dir.dir_original = '>';
    c[6].dir.dir_target = '^';
    c[7].dir.dir_original = '<';
}

carThread cars[8];

// get direction as index
int dirInt(char d){
    if (d == '<')
        return 0;
    else if (d == '^')
        return 1;
    else if (d == '>')
        return 2;
    else
        return 3;
}

// get motion of car
char motion(directions dir){
    char ori = dir.dir_original;
    char tar = dir.dir_target;
    if(ori == tar)
        return 's';
    else if (ori == '^'){
        if (tar == '<')
            return 'l';
        else if (tar == '>')
            return 'r';
    }
    else if (ori == '>'){
        if (tar == '^')
            return 'l';
        else if (tar == 'v')
            return 'r';
    }
    else if (ori == 'v'){
        if (tar == '>')
            return 'l';
        else if (tar == '<')
            return 'r';
    }
    else{ // tar == <
        if (tar == 'v')
            return 'l';
        else
            return 'r';
    }

    return ' ';
}

// get motion as an index
int motionInt(char m){
    if (m == 'l')
        return 0;
    else if (m == 's')
        return 1;
    else
        return 2;
}

void ArriveIntersection(directions dir){
    sem_wait(&lock);          /* lock the front         */
    if (curr_d == ' ')
        curr_d = dir.dir_original;
    char m = motion(dir);       // get motion
    char ori = dir.dir_original;
    int m_int = motionInt(m);
    int ori_int = dirInt(ori);
    if (ori == curr_d)
        curr_m[m_int]++;
    sem_post(&lock);         /* release front          */
    if(ori == curr_d){
        if (curr_m[m_int] == 1)
            sem_wait(&d[ori_int][m_int]);
        else if (curr_m[m_int] > 1) // follows same motion a previous
            return;

        if (m == 'l'){ // wait for those that don't match
            sem_wait(&d[(ori_int+2) % 4][1]);
            sem_wait(&d[(ori_int+2) % 4][2]);
            if (curr_m[1] < 1){
                sem_wait(&d[(ori_int+1) % 4][0]);
                sem_wait(&d[(ori_int+3) % 4][0]);
                sem_wait(&d[(ori_int+3) % 4][1]);
            
                if (curr_m[2] < 1){
                    sem_wait(&d[(ori_int+1) % 4][1]);
                }
            }
        }
        else if (m == 's'){ // wait for those that don't match
            sem_wait(&d[(ori_int+3) % 4][2]);
            if (curr_m[0] < 1){
                sem_wait(&d[(ori_int+1) % 4][0]);
                sem_wait(&d[(ori_int+3) % 4][0]);
                sem_wait(&d[(ori_int+3) % 4][1]);
            
                if (curr_m[2] < 1){
                    sem_wait(&d[(ori_int+1) % 4][1]);
                    sem_wait(&d[(ori_int+2) % 4][0]);
                }
            }
            else if (curr_m[2] < 1){
                    sem_wait(&d[(ori_int+2) % 4][0]);
            }
        }
        else {
            if (curr_m[1] < 1){
                if (curr_m[0] < 1){
                    sem_wait(&d[(ori_int+1) % 4][1]);
                }
                else{
                    sem_wait(&d[(ori_int+2) % 4][0]);
                }
            }
        }
    }
    else {
        sem_wait(&newD);
        if (m == 'l'){ // wait for those that don't match
            sem_wait(&d[(ori_int+2) % 4][1]);
            sem_wait(&d[(ori_int+2) % 4][2]);
            sem_wait(&d[(ori_int+1) % 4][0]);
            sem_wait(&d[(ori_int+3) % 4][0]);
            sem_wait(&d[(ori_int+3) % 4][1]);
            sem_wait(&d[(ori_int+1) % 4][1]);
        }
        else if (m == 's'){ // wait for those that don't match
            sem_wait(&d[(ori_int+3) % 4][2]);
            sem_wait(&d[(ori_int+1) % 4][0]);
            sem_wait(&d[(ori_int+3) % 4][0]);
            sem_wait(&d[(ori_int+3) % 4][1]);
            sem_wait(&d[(ori_int+1) % 4][1]);
            sem_wait(&d[(ori_int+2) % 4][0]);

        }
        else {
            sem_wait(&d[(ori_int+1) % 4][1]);
            sem_wait(&d[(ori_int+2) % 4][0]);
        }
        sem_wait(&lock);
        curr_m[0] = 0;
        curr_m[1] = 0;
        curr_m[2] = 0;
        curr_m[m_int]++;
        curr_d = ori;
        sem_post(&lock);
        sem_post(&newD);
    }
}

void CrossIntersection(directions dir){
    char m = motion(dir);
    if (m == 's')
        Spin(tStright);
    else if (m == 'l')
        Spin(tLeft);
    else
        Spin(tRight);
}

void ExitIntersection(directions dir){
    sem_wait(&lock);
    char m = motion(dir);
    char ori = dir.dir_original;
    int m_int = motionInt(m);
    int ori_int = dirInt(ori);
    if (ori == curr_d)
        curr_m[m_int]--;

    if (m == 'l'){ // post for those that don't match
            if (curr_m[0] < 1){
                sem_post(&d[(ori_int+2) % 4][1]);
                sem_post(&d[(ori_int+2) % 4][2]);
            }
            if (curr_m[1] < 1){
                sem_post(&d[(ori_int+1) % 4][0]);
                sem_post(&d[(ori_int+3) % 4][0]);
                sem_post(&d[(ori_int+3) % 4][1]);
            
                if (curr_m[2] < 1){
                    sem_post(&d[(ori_int+1) % 4][1]);
                }
            }
    }
    else if (m == 's'){ // wait for those that don't match
        if (curr_m[1] < 1)
            sem_post(&d[(ori_int+3) % 4][2]);
        if (curr_m[0] < 1){
            sem_post(&d[(ori_int+1) % 4][0]);
            sem_post(&d[(ori_int+3) % 4][0]);
            sem_post(&d[(ori_int+3) % 4][1]);
        
            if (curr_m[2] < 1){
                sem_post(&d[(ori_int+1) % 4][1]);
                sem_post(&d[(ori_int+2) % 4][0]);
            }
        }
    }
    else {
        if (curr_m[1] < 1){
            if (curr_m[0] < 1){
                sem_post(&d[(ori_int+1) % 4][1]);
            }
            else{
                sem_post(&d[(ori_int+2) % 4][0]);
            }
        }
    }
    sem_post(&lock);

}

// Car subroutine
void Car(carThread c) {
    double t_start = GetTime();
    double t;
    directions dir = c.dir;

    usleep(c.arrivalTime*1000000);
    t = GetTime() - t_start;
    printf("Time %.1f: Car %d '('%c %c')' arriving\n", t, c.cid, dir.dir_original, dir.dir_target);
    ArriveIntersection(dir);

    usleep(2000000);
    t = GetTime() - t_start;
    printf("Time %.1f: Car %d '('%c %c')' \t crossing\n", t, c.cid, dir.dir_original, dir.dir_target);
    CrossIntersection(dir);

    t = GetTime() - t_start;
    printf("Time %.1f: Car %d '('%c %c')' \t\t exiting\n", t, c.cid, dir.dir_original, dir.dir_target);
    ExitIntersection(dir);
}

void *child(void *arg){
    int index = *(int*) arg;

    carThread c = cars[index];
    Car(c);

    free(arg);
    return NULL;
}

int main(int argc, char *argv[]) {
    carsInit(cars);

    // initialize d, only to be used with threads in this process, set value to 1
    for(int i = 0; i< 4; i++){
       for(int j = 0; j < 3; j++){
           sem_init(&d[i][j], 0, 1);
       }
    }
    sem_init(&lock, 0, 1);
    sem_init(&newD, 0, 1);
    
    pthread_t p[8];

    int i;
    for (i = 0; i < 8; i++) {
        int* a = malloc(sizeof(int));
        *a = i;
        Pthread_create(&p[i], NULL, child, a);
    }
    

    for (i = 0; i < 8; i++) 
	Pthread_join(p[i], NULL);

    return 0;
}
