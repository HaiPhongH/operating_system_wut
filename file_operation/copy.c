#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main (int argc, char *argv[]) {
    
    char *filename1;        // source file
    char *filename2;        // destination file

    int fd;                 // file descriptor
    int opt;                // command line options
    int map_file = 0;       // check using mapping file or nor
    struct stat sb;         // get file status

    while ((opt = getopt(argc, argv, ":hm:")) != -1) {
        switch (opt) {
            case 'h':
                printf("USAGE.\n");
                printf("Command for map files to memory regions, and do the copy after that.\n");
                printf("    copy -m source_file destination_file.\n");
                printf("    Ex: copy -m new_file.txt test_m.txt\n");
                printf("Command for copy file in the original way.\n");
                printf("    copy source_file destination_file.\n");
                printf("    Ex: copy new_file.txt test.txt\n");
                printf("Command for display the usage.\n");
                printf("    copy -h.\n");
                
                return 0;
            // case using mapping file
            case 'm':
                map_file = 1;
                filename1 = argv[optind-1];    // get the name of source file from command line 
                filename2 = argv[optind];      // get the name of destination file from command line 
                break;
            // case unknown options
            case '?':
                printf("Uknown options %c.\n", optopt);
                break;
        }
    }
    
    // using original way to copy file (use read() and write() function)
    if (map_file == 0) {
        filename1 = argv[1];    // get the name of source file from command line 
        filename2 = argv[2];    // get the name of destination file from command line 
        
        // code section for reading file
        fd = open(filename1, O_RDONLY);     // open the source file
        
        // check value of file descriptor
        if (fd == -1) {
            printf("Errors occurs while reading file.\n");
            exit(1);
        }
        // check the value of function getting file status
        if (fstat(fd, &sb) == -1) {
            printf("fstat error.\n");
            exit(1);
        }

        // check empty file
        if (sb.st_size == 0) {
            printf("Empty file.\n");
            exit(1);
        }

        // declare a buffer to read data from source file
        char buffers[sb.st_size];
        // read data
        read(fd, buffers, sb.st_size);
        // end of the file
        buffers[sb.st_size] = '\0';

        // code section for writing to file
        // open destination file
        fd = open(filename2, O_CREAT | O_WRONLY, 0600);

        // check the value of file descriptor
        if (fd == -1) {
            printf("Errors occurs while writing file.\n");
            exit(1);
        }

        // write buffer to the destination file
        write(fd, buffers, sb.st_size);
    }

    // Using mapping file (do not use neither read() or write() function)
    else {
        // open the source file
        fd = open(filename1, O_RDONLY);
        // check the value of file descriptor
        if (fd == -1) {
            printf("Errors occurs while reading file.\n");
            exit(1);
        }

        // get the size of source file in bytes
        int len = lseek(fd, 0, SEEK_END);
        // check the empty file
        if (len == 0 ){
            printf("Empty file.\n");
            exit(0);
        }

        // map source file into memory
        char *source_data = mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
        // check the mapping is success or not
        if (source_data == MAP_FAILED) {
            printf("Source: Map failed.\n");
            exit(1);
        }

        // open the destination file
        fd = open(filename2, O_RDWR | O_CREAT | O_TRUNC, 0600);
        // check the value of file descriptor
        if (fd == -1) {
            printf("Errors occurs while reading file.\n");
            exit(1);
        }

        // truncate the destination file to a specified length (value of len)
        if (ftruncate(fd, len) == -1) {
            printf("Destination: Truncate error.\n");
            exit(1);
        }

        // map destination file into memory
        char *destination_data = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        // check the mapping is success or not
        if (destination_data == MAP_FAILED) {
            printf("Destination: Map failed.\n");
            exit(1);
        }
        // copy data from source to destination
        memcpy(destination_data, source_data, len); 

        // un map the source and the destination file
        munmap(source_data, len);
        munmap(destination_data, len);
    }

    // close the file
    close(fd);
    return 0;
}