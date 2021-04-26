#define MAX_BSEM 128

struct binary_semaphore {
    int occupied;
    int value;
};

struct binary_semaphore Bsemaphores[MAX_BSEM];

int bsem_alloc() {
    int descriptor;
    int found;
    for (descriptor = 0; !found && descriptor < MAX_BSEM; descriptor++) {
        if(!Bsemaphores[descriptor].occupied){
            found = 1;
            break;
        }
    }
    if (!found)
        return -1;
    Bsemaphores[descriptor].occupied = 1;
    Bsemaphores[descriptor].value = 1;
    return descriptor;
}

void bsem_free(int descriptor) {
    Bsemaphores[descriptor].occupied = 0;
    Bsemaphores[descriptor].value = 1;
}

void bsem_down(int descriptor) {
    if (Bsemaphores[descriptor].occupied) {
        Bsemaphores[descriptor].value = 0;
    }
    else {
        //sleep on 
    }
}

void bsem_up(int descriptor) {
    if (!Bsemaphores[descriptor].occupied) {
        Bsemaphores[descriptor].value = 1;
        //wakeup
    }
}