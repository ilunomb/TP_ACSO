#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "file.h"
#include "inode.h"
#include "diskimg.h"

/**
 * TODO
 */
int file_getblock(struct unixfilesystem *fs, int inumber, int blockNum, void *buf) {
    struct inode in;
    if (inode_iget(fs, inumber, &in) < 0) return -1;

    int disk_block = inode_indexlookup(fs, &in, blockNum);
    if (disk_block < 0) return -1;

    int bytes_read = diskimg_readsector(fs->dfd, disk_block, buf);
    if (bytes_read != DISKIMG_SECTOR_SIZE) return -1;

    int file_size = inode_getsize(&in);
    int offset = blockNum * DISKIMG_SECTOR_SIZE;

    if (offset >= file_size) return 0;

    int remaining = file_size - offset;
    return (remaining < DISKIMG_SECTOR_SIZE) ? remaining : DISKIMG_SECTOR_SIZE;
}


