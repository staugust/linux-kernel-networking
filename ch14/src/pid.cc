#include <iostream>
#include <sched.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <sys/wait.h>

/*
 * Check the difference for unshare and fork with pid namespace
 */
int main(int argc, char * argv[]) {
    printf("self pid %d\n", getpid());
    pid_t self ;
    pid_t ch1, ch2;
    char  * params[] = {"/usr/bin/lsns", "-p", NULL, NULL};
    char * pstr;
    int idx, ret;
    params[2] = (char *)malloc(sizeof(char) * 32);
    pstr = params[2];
    memset(pstr, 0, 32);
    sprintf(pstr, "%d", self );
    ch1 = fork();
    if(ch1 == 0){
        self = getpid();
        printf("this is child %d:%d\n", getppid(), self);
        memset(pstr, 0, 32);
        sprintf(pstr, "%d", self );
        ret = execv(params[0], params);
        if(ret != 0) {
            std::cout << errno << std::endl;
        }
        return 0;
    }
    sleep(1);
    ret = unshare(CLONE_NEWPID);
    if(ret != 0) {
        std::cout << "unshare: " << errno << std::endl;
    }
    ch2 = fork();
    if(ch2 == 0){
        self = getpid();
        printf("this is child2 %d:%d\n", getppid(), self);
        memset(pstr, 0, 32);
        sprintf(pstr, "%d", self );
        ret = execv(params[0], params);
        if(ret != 0) {
            std::cout << errno << std::endl;
        }
        return 0;
    }
    sleep(1);
    self = getpid();
    printf("this is parent %d\n", self);
    memset(pstr, 0, 32);
    sprintf(pstr, "%d", self );
    ret = execv(params[0], params);
    if(ret != 0) {
        std::cout << errno << std::endl;
    }
    wait(NULL);
    wait(NULL);
    return 0;
}