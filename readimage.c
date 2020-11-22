#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "ext2.h"

unsigned char *disk;

void print_bitmap(char *bits, unsigned int len) {
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
    int fd = open(argv[1], O_RDWR);

    disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    struct ext2_super_block *sb = (struct ext2_super_block *)(disk + 1024);
    printf("Inodes: %d\n", sb->s_inodes_count);
    printf("Blocks: %d\n", sb->s_blocks_count);
    printf("Block group:\n");
    // get the descriptor table in the second block
    struct ext2_group_desc *db = (struct ext2_group_desc*) (disk + EXT2_BLOCK_SIZE*2);
    printf("    block bitmap: %d\n", db->bg_block_bitmap);
    printf("    inode bitmap: %d\n", db->bg_inode_bitmap);
    printf("    inode table: %d\n", db->bg_inode_table);
    printf("    free blocks: %d\n", db->bg_free_blocks_count);
    printf("    free inodes: %d\n", db->bg_free_inodes_count);
    printf("    used_dirs: %d\n", db->bg_used_dirs_count);

    unsigned char *block_bitmap = (unsigned char *) (disk + EXT2_BLOCK_SIZE*3);
    printf("Block bitmap: ");
    print_bitmap(block_bitmap, sb->s_blocks_count / 8);
    unsigned char *inode_bitmap = (unsigned char *) (disk + EXT2_BLOCK_SIZE*4);
    printf("Inode bitmap: ");
    print_bitmap(inode_bitmap, sb->s_inodes_count / 8);
    printf("\n");

    struct ext2_inode *inode_table = (struct ext2_inode*) (disk + EXT2_BLOCK_SIZE*db->bg_inode_table);

    // print the root first
    struct ext2_inode root = inode_table[EXT2_ROOT_INO - 1];
    printf("Inodes:\n");
    printf("[%d] type: %c size: %d links: %d blocks %d\n", EXT2_ROOT_INO, 'd', root.i_size, root.i_links_count, root.i_blocks);
    printf("[%d] Blocks:", EXT2_ROOT_INO);
    for (int i = 0; i < root.i_blocks/(2<<sb->s_log_block_size); ++i) {
        printf(" %u", root.i_block[i]);
    }
    printf("\n");


    unsigned int occupied_count = sb->s_inodes_count - sb->s_free_inodes_count;
    char file_type;
    for (int i = EXT2_GOOD_OLD_FIRST_INO; i < occupied_count; ++i) {
        if (inode_table[i].i_mode & S_IFLNK) {
            file_type = '?'; // Char for links??? TODO
        } else if (inode_table[i].i_mode & EXT2_S_IFREG) {
            file_type = 'f';
        } else {
            file_type = 'd';
        }

        printf("[%d] type: %c size: %d links: %d blocks %d\n", i + 1, file_type, inode_table[i].i_size, inode_table[i].i_links_count, inode_table[i].i_blocks);
        printf("[%d] Blocks:", i + 1);
        for (int j = 0; j < inode_table[i].i_blocks/(2<<sb->s_log_block_size); ++j) {
            printf(" %u", inode_table[i].i_block[j]);
        }
        printf("\n");
    }


    return 0;
}
