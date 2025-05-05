#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "inode.h"
#include "diskimg.h"


/**
 * TODO
 */
int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {
    if (inumber <= 0) return -1;

    int inodes_per_block = DISKIMG_SECTOR_SIZE / sizeof(struct inode);
    int sector = INODE_START_SECTOR + (inumber - 1) / inodes_per_block;
    int offset = (inumber - 1) % inodes_per_block;

    struct inode buf[inodes_per_block];
    int res = diskimg_readsector(fs->dfd, sector, buf);
    if (res != DISKIMG_SECTOR_SIZE) return -1;

    *inp = buf[offset];
    return 0;
}


/**
 * TODO
 */
int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp, int blockNum) {
    if (!(inp->i_mode & IALLOC)) return -1;

    // No es archivo "large" → bloques directos
    if ((inp->i_mode & ILARG) == 0) {
        if (blockNum < 0 || blockNum >= 8) return -1;
        return inp->i_addr[blockNum];
    }

    // Es archivo "large"
    if (blockNum < 0 || blockNum >= 7 * 256 + 256 * 256) return -1;

    uint16_t blockbuf[256];

    // Bloques indirectos (primeros 7)
    if (blockNum < 7 * 256) {
        int indirect_index = blockNum / 256;
        int inner_index = blockNum % 256;

        int indirect_block = inp->i_addr[indirect_index];
        if (diskimg_readsector(fs->dfd, indirect_block, blockbuf) != DISKIMG_SECTOR_SIZE) return -1;

        return blockbuf[inner_index];
    }

    // Bloques doblemente indirectos
    int double_index = blockNum - 7 * 256;
    int first_level = double_index / 256;
    int second_level = double_index % 256;

    int doubly_indirect_block = inp->i_addr[7];
    if (diskimg_readsector(fs->dfd, doubly_indirect_block, blockbuf) != DISKIMG_SECTOR_SIZE) return -1;

    int indirect_block = blockbuf[first_level];
    if (diskimg_readsector(fs->dfd, indirect_block, blockbuf) != DISKIMG_SECTOR_SIZE) return -1;

    return blockbuf[second_level];
}


int inode_getsize(struct inode *inp) {
  return ((inp->i_size0 << 16) | inp->i_size1); 
}
