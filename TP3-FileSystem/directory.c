#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include "file.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

/**
 * TODO
 */
int directory_findname(struct unixfilesystem *fs, const char *name,
  int dirinumber, struct direntv6 *dirEnt) {

struct inode dir_inode;
if (inode_iget(fs, dirinumber, &dir_inode) < 0) return -1;

int dir_size = inode_getsize(&dir_inode);
int num_blocks = (dir_size + DISKIMG_SECTOR_SIZE - 1) / DISKIMG_SECTOR_SIZE;

char block[DISKIMG_SECTOR_SIZE];

for (int i = 0; i < num_blocks; i++) {
  int nbytes = file_getblock(fs, dirinumber, i, block);
  if (nbytes < 0) return -1;

  int nentries = nbytes / sizeof(struct direntv6);
  struct direntv6 *entry = (struct direntv6 *)block;

  for (int j = 0; j < nentries; j++) {
    if (strncmp(entry[j].d_name, name, 14) == 0) {
      *dirEnt = entry[j];
      return 0;
    }
  }
}

return -1; // Not found
}
