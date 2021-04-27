#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/syscall.h"
#include "kernel/memlayout.h"
#include "kernel/riscv.h"

struct sigaction {
  void (*sa_handler) (int);
  uint sigmask;
};

void sig_handler_1() {
    printf("HELLO IM HANDLER #1\n");
}

void sig_handler_2() {
    printf("HELLO IM HANDLER 2\n");
}

int
test1_check_sigaction_mask_update() {
    int test_status = 0;
    struct sigaction* sig_action_1 = malloc(sizeof(struct sigaction*));
    struct sigaction* sig_action_2 = malloc(sizeof(struct sigaction*));
    sig_action_1->sa_handler = &sig_handler_1;
    sig_action_1->sigmask = 1;

    if(sigaction(2, sig_action_1, 0) == -1){
        printf("test1_check_sigaction_mask_update failed - syscall returned -1\n");
        test_status = 1;
    }

    sigaction(2, 0, sig_action_2);

    if(sig_action_2->sigmask != 1){
        printf("test1_check_sigaction_mask_update failed - sigmask was not updated\n");
        test_status = 1;
    }

    sig_action_1->sa_handler = (void*)SIG_DFL;
    sig_action_1->sigmask = 0;
    for(int i=0; i<NSIGS; i++){
        sigaction(i, sig_action_1, 0);
    }
   return test_status;
}

int
test2_check_sigaction_mask_failure() {
    int test_status = 0;
    struct sigaction* sig_action_1 = malloc(sizeof(struct sigaction*));
    sig_action_1->sigmask = (1 << SIGSTOP);
    if(sigaction(2, sig_action_1, 0) >= 0){
        test_status = 1;
        printf("test2_check_sigaction_mask_failure failed - syscall was supposed to return -1\n");
    }
    sig_action_1->sigmask = (1 << SIGKILL);
    if(sigaction(2, sig_action_1, 0) >= 0){
        test_status = 1;
        printf("test2_check_sigaction_mask_failure failed - syscall was supposed to return -1\n");
    }
    sig_action_1->sigmask = 0;
    if(sigaction(-2, sig_action_1, 0) >= 0){
        test_status = 1;
        printf("test2_check_sigaction_mask_failure failed - syscall was supposed to return -1\n");
    }
    if(sigaction(63, sig_action_1, 0) >= 0){
        test_status = 1;
        printf("test2_check_sigaction_mask_failure failed - syscall was supposed to return -1\n");
    }
    if(sigaction(SIGSTOP, sig_action_1, 0) >= 0){
        printf("test2_check_sigaction_mask_failure failed - syscall was supposed to return -1\n");
    }
    if(sigaction(SIGKILL, sig_action_1, 0) >= 0){
        test_status = 1;
        printf("test2_check_sigaction_mask_failure failed - syscall was supposed to return -1\n");
    }

    sig_action_1->sa_handler = (void*)SIG_DFL;
    sig_action_1->sigmask = 0;
    for(int i=0; i<NSIGS; i++){
        sigaction(i, sig_action_1, 0);
    }
    return test_status;
}

int 
test3_check_sigprocmask() {
    
    int test_status = 0;
    uint first_mask = sigprocmask((1 << 4) | (1 << 3));
    if(first_mask != 0){
        printf("test3_check_sigprocmask failed - first_mask is not previous mask!\n");
        test_status = 1;
    }

    uint second_mask = sigprocmask((1 << 2) | (1 << 3) | (1 << 5));
    if(second_mask != ((1 << 4) | (1 << 3))){
        printf("test3_check_sigprocmask failed - second_mask is not previous mask!\n");
        test_status = 1;
    }

    int pid = fork();
    if(pid == 0){
        uint child_first_mask = sigprocmask((1 << 7) | (1 << 6));
        if(child_first_mask != ((1 << 2) | (1 << 3) | (1 << 5))){
            printf("test3_check_sigprocmask failed - child_first_mask is not previous mask!\n");
            test_status = 1;
        } else {
            char* argv[] = {"test", "sigprocmask", 0};
            if(exec(argv[0],argv)<0){
                printf("test3_check_sigprocmask failed - could not exec!\n");
                test_status = 1;
                exit(1);
            }
        }
    }
    int status;
    wait(&status);
    sigprocmask(0);
    return (test_status & status);
}

int test4_check_sigaction_handler_update() {
    int test_status = 0;
    struct sigaction* sig_action_1 = malloc(sizeof(struct sigaction*));
    struct sigaction* sig_action_2 = malloc(sizeof(struct sigaction*));
    sig_action_1->sa_handler = &sig_handler_1;
    sig_action_1->sigmask = 1;
    
    if(sigaction(2, sig_action_1, sig_action_2) < 0){
        printf("test4_check_sigaction_handler_update - first sigaction failed!\n");
        test_status = 1;
    }

    if(sig_action_2->sa_handler != (void*)SIG_DFL){
        printf("test4_check_sigaction_handler_update - sig_action_2 prev handler not DFL!\n");
        test_status = 1;
    }

    if(sigaction(2, 0, sig_action_2) < 0){
        printf("test4_check_sigaction_handler_update - second sigaction failed!\n");
        test_status = 1;
    }

    if(sig_action_2->sa_handler != &sig_handler_1){
        printf("test4_check_sigaction_handler_update - sig_action_2 prev handler not sig_handler_1!\n");
        test_status = 1;
    }

    sig_action_1->sa_handler = &sig_handler_2;
    sigaction(3, sig_action_1, 0);
    sig_action_1->sa_handler = (void*)SIG_IGN;
    sigaction(4, sig_action_1, 0);

    int pid = fork();
    if (pid == 0) {
        struct sigaction* sig_action_3 = malloc(sizeof(struct sigaction*));
        sigaction(2, 0, sig_action_3);
        if (sig_action_3->sa_handler != &sig_handler_1) {
            printf("test4_check_sigaction_handler_update - child did not inherit signal 2 handler!\n");
            test_status = 1;
        }
        sigaction(3, 0, sig_action_3);
        if (sig_action_3->sa_handler != &sig_handler_2) {
            printf("test4_check_sigaction_handler_update - child did not inherit signal 3 handler!\n");
            test_status = 1;
        }
        sigaction(4, 0, sig_action_3);
        if (sig_action_3->sa_handler != (void*)SIG_IGN) {
            printf("test4_check_sigaction_handler_update - child did not inherit signal 4 handler!\n");
            test_status = 1;
        }
        char* argv[] = {"test", "sigaction_handler", 0};
        if(exec(argv[0],argv)<0){
            printf("test4_check_sigaction_handler_update failed - could not exec!\n");
            test_status = 1;
            exit(1);
        }
    }
    int status;
    wait(&status);
    sig_action_1->sa_handler = (void*)SIG_DFL;
    sig_action_1->sigmask = 0;
    for(int i=0; i<NSIGS; i++){
        sigaction(i, sig_action_1, 0);
    }
    return test_status & status;
}


void test5_check_sending_signals() {
    
}

int
main(int argc, char *argv[]){

    struct test {
    int (*f)();
    char *s;
    } tests[] = {
        {test1_check_sigaction_mask_update, "test1_check_sigaction_mask_update"},
        {test2_check_sigaction_mask_failure, "test2_check_sigaction_mask_failure"},
        {test3_check_sigprocmask, "test3_check_sigprocmask"},
        {test4_check_sigaction_handler_update, "test4_check_sigaction_handler_update"},
        {0,0}
    };

    if(argc>1){
        if(strcmp(argv[1],"sigprocmask") == 0){
            uint exec_mask = sigprocmask(0);
            if(exec_mask != ((1 << 7) | (1 << 6))){
                printf("%d", exec_mask);
                printf("test3_check_sigprocmask failed - exec_mask is not previous value!\n");
                exit(1);
            }
            exit(0);
        }
        if(strcmp(argv[1],"sigaction_handler") == 0){
            struct sigaction* sig_action_3 = malloc(sizeof(struct sigaction*));
            sigaction(2, 0, sig_action_3);
            if (sig_action_3->sa_handler != (void*)SIG_DFL) {
                printf("test4_check_sigaction_handler_update - exec did not initialize signal 2 handler!\n");
                exit(1);
            }
            sigaction(3, 0, sig_action_3);
            if (sig_action_3->sa_handler != (void*)SIG_DFL) {
                printf("test4_check_sigaction_handler_update - exec did not initialize signal 3 handler!\n");
                exit(1);
            }
            sigaction(4, 0, sig_action_3);
            if (sig_action_3->sa_handler != (void*)SIG_IGN) {
                printf("test4_check_sigaction_handler_update - exec did not initialize signal 4 handler!\n");
                exit(1);
            } 
            exit(0);
        }
    }
    for (struct test *t = tests; t->s != 0; t++) {
        printf("%s: ", t->s);
        int test_status = (t->f)();
        if(!test_status) printf("OK\n");
        else printf("FAILED!\n");
    }
    exit(0);  
}

