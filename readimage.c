#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "ext2.h"

unsigned char *disk;

char get_inode_type(struct ext2_inode *inode_table, int index) {
    char file_type;
    if (inode_table[index].i_mode & EXT2_S_IFLNK) {
        file_type = '0';
    } 
    if (inode_table[index].i_mode & EXT2_S_IFREG) {
        file_type = 'f';
    } else {
        file_type = 'd';
    }

    return file_type;
}

void print_bitmap(unsigned char *bits, unsigned int len) {
    // print out the char (1 byte) one by one
    for (int i = 0; i < len; ++i){
        for (int j = 0; j < 8; ++j) {
            printf("%d", (bits[i] >> j) & 1);
        }
        printf(" ");
    }
    printf("\n");
}

int main(int argc, char **argv) {

    if(argc != 2) {
        fprintf(stderr, "Usage: %s <image file name>\n", argv[0]);
        exit(1);
    }

    int counter;
    char* p = strdup(argv[1]);
    char* token;

    while (token = strtok_r(p, "/", &p)) {
        counter++;
    }
    
    counter--;

    printf("%d\n", counter);
    return 0;
}
