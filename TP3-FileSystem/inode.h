#ifndef _INODE_H
#define _INODE_H

#include "unixfilesystem.h"

/**
 * Fetches the specified inode from the filesystem. 
 * Returns 0 on success, -1 on error.  
 */
int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp); 

/**
 * Given an index of a file block, retrieves the file's actual block number
 * of from the given inode.
 *
 * Returns the disk block number on success, -1 on error.  
 */
int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp, int blockNum);

/**
 * Computes the size in bytes of the file identified by the given inode
 */
int inode_getsize(struct inode *inp);

int inode_is_allocated(struct inode *in);
int inode_is_large(struct inode *in);
int inode_is_directory(struct inode *in);


#endif // _INODE_
