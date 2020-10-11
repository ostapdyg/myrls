



int fd = shm_open(name, O_RDWR, mode);


void* shared_mem = mmap(NULL, sh_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);


sem_t* sem_fd = sem_open(sem_name, 0, mode);
sem_wait(sem_fd);
// while(!*shared_mem);

char* buf[sh_size];

char* buf_c = buf;

for(char* c= (char*) (shared_mem+1), (*c), c++){
    *buf_c = *c;
    *buf_c++;
}
printf("%s", buf);

*shared_mem = 0;


sem_close(sem_fd);

// ---------------------------------------

int fd = shm_open(name, O_RDWR, mode);


void* shared_mem = mmap(NULL, sh_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);


char* buf = "Hello, world";

char*buf_c = buf;

for(char* c= (char*) (shared_mem+1), (*c), c++){
    *c = *buf_c;
    *buf_c++;
}
*(c+1) = 0;


sem_t* sem_fd = sem_open(sem_name, 0, mode);
sem_post(sem_fd);

sem_close(sem_fd);
