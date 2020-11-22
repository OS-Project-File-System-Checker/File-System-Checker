#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define stat xv6_stat  // avoid clash with host struct stat
#include "types.h"
#include "fs.h"
#include "stat.h"
#include "param.h"
#define SUPERBLOCK 1
#define T_ERR 4



int fsfd;
struct superblock sb;


ushort
xshort(ushort x)
{
    ushort y;
    uchar *a = (uchar*)&y;
    a[0] = x;
    a[1] = x >> 8;
    return y;
}

uint
xint(uint x)
{
    uint y;
    uchar *a = (uchar*)&y;
    a[0] = x;
    a[1] = x >> 8;
    a[2] = x >> 16;
    a[3] = x >> 24;
    return y;
}

void
rsect(uint sec, void *buf)
{
    if(lseek(fsfd, sec * BSIZE, 0) != sec * BSIZE){
        perror("lseek");
        exit(1);
    }
    if(read(fsfd, buf, BSIZE) != BSIZE){
        perror("read");
        exit(1);
    }
}

void
rinode(uint inum, struct dinode *ip)
{
    char buf[BSIZE];
    uint bn;
    struct dinode *dip;

    bn = IBLOCK(inum, sb);
    rsect(bn, buf);
    dip = ((struct dinode*)buf) + (inum % IPB);
    *ip = *dip;
}

void
wsect(uint sec, void *buf)
{
    if(lseek(fsfd, sec * BSIZE, 0) != sec * BSIZE){
        perror("lseek");
        exit(1);
    }
    if(write(fsfd, buf, BSIZE) != BSIZE){
        perror("write");
        exit(1);
    }
}

void
winode(uint inum, struct dinode *ip)
{
    char buf[BSIZE];
    uint bn;
    struct dinode *dip;

    bn = IBLOCK(inum, sb);
    rsect(bn, buf);
    dip = ((struct dinode*)buf) + (inum % IPB);
    *dip = *ip;
    wsect(bn, buf);
}



int diskinfo(int fsfd) {        //function that prints the details of the superblock
    uchar buf[BSIZE];
    rsect(SUPERBLOCK,buf);
    memmove(&sb, buf, sizeof(sb));

    printf("bitmap start: %d\n",sb.bmapstart);
    printf("inode start: %d\n",sb.inodestart);
    printf("log start : %d\n",sb.logstart);
    printf("nblocks : %d\n",sb.nblocks);
    printf("ninodes : %d\n",sb.ninodes);
    printf("nlog : %d\n",sb.nlog);
    printf("size : %d\n",sb.size);
    return 0;
}

////////////////////////////////////// start errors from here ////////////////////////////////////////////

int err1(int fsfd){
    uchar buf[BSIZE];
    rsect(SUPERBLOCK,buf);
    memmove(&sb, buf, sizeof(sb));

    int inum = 1;
    struct dinode current_inode;

    if (lseek(fsfd, sb.inodestart * BSIZE + inum * sizeof(struct dinode), SEEK_SET) != sb.inodestart * BSIZE + inum * sizeof(struct dinode)){  //move to correct location
            perror("lseek");
            exit(1);
        }
        if(read(fsfd, buf, sizeof(struct dinode))!=sizeof(struct dinode)){	//read inode info into buffer
            perror("read");
            exit(1);
        }

    memmove(&current_inode, buf, sizeof(current_inode));
    current_inode.type = T_ERR;

    write(fsfd,&current_inode,sizeof(current_inode));
    
    printf("Changed the type of a inode");

    return 1;
}

int err2(int fsfd){
    uchar buf[BSIZE];
    rsect(SUPERBLOCK,buf);
    memmove(&sb, buf, sizeof(sb));

    int inum = 5;
    struct dinode current_inode;

    if (lseek(fsfd, sb.inodestart * BSIZE + inum * sizeof(struct dinode), SEEK_SET) != sb.inodestart * BSIZE + inum * sizeof(struct dinode)){  //move to correct location
            perror("lseek");
            exit(1);
        }
        if(read(fsfd, buf, sizeof(struct dinode))!=sizeof(struct dinode)){	//read inode info into buffer
            perror("read");
            exit(1);
        }

    memmove(&current_inode, buf, sizeof(current_inode));
    current_inode.addrs[0] = 100000;
	current_inode.addrs[11] = 100000;

    write(fsfd,&current_inode,sizeof(current_inode));

    return 1;
}

int err3(int fsfd){
    uint b = 0;
    write(fsfd,&b,1);

    if(lseek(fsfd, 59*BSIZE, 0) != 59 * BSIZE){
        perror("lseek");
        exit(1);
    }

    
    return 1;
}

int err4(int fsfd){
    uint b = 10;
    write(fsfd,&b,1);

    if(lseek(fsfd, 59*BSIZE+1, 0) != 59 * BSIZE+1){
        perror("lseek");
        exit(1);
    }
    return 1;
}

int err5(int fsfd){
	uchar buf[BSIZE];
    rsect(SUPERBLOCK,buf);
    memmove(&sb, buf, sizeof(sb));
	
    uint buf2;
    rinode(10,&inode1);
    uint bmapbyte;

    int inodeaddr = inode1.addrs[1]/4;

    if(lseek(fsfd, sb.bmapstart*BSIZE + inodeaddr, 0) != sb.bmapstart * BSIZE  + inodeaddr){
        perror("lseek");
        exit(1);
    }

    uint a = 0;

    if (write(fsfd,&a,1) != 1){
        perror("write");
        exit(1);
    }

    printf("Used inode marked free in bitmap");
    return 1;

}

int err6(int fsfd){
	uchar buf[BSIZE];
    rsect(SUPERBLOCK,buf);
    memmove(&sb, buf, sizeof(sb));
	
    uint bmapbyte;

    if(lseek(fsfd, sb.bmapstart*BSIZE + 100, 0) != sb.bmapstart * BSIZE  + 100){
        perror("lseek");
        exit(1);
    }

    uint a = 255;

    if (write(fsfd,&a,1) != 1){
        perror("write");
        exit(1);
    }

    return 1;

}

int err9(int fsfd){
  uchar buf[BSIZE];
	rsect(SUPERBLOCK,buf);
	memmove(&sb, buf, sizeof(sb));

  int inum;
  struct dinode current_inode;
  for (inum=0; inum<sb.ninodes; inum++){	
  if (lseek(fsfd, sb.inodestart * BSIZE + inum * sizeof(struct dinode), SEEK_SET) != sb.inodestart * BSIZE + inum * sizeof(struct dinode)){  //move to correct location
	  perror("lseek");
    exit(1);
	}
	if(read(fsfd, buf, sizeof(struct dinode))!=sizeof(struct dinode)){	//read inode info into buffer
	  perror("read");
    exit(1);
	}
  memmove(&current_inode, buf, sizeof(current_inode));
  if (current_inode.type==0){
    current_inode.type = T_FILE;
    write(fsfd,&current_inode,sizeof(current_inode));
    printf("marked a free inode as used Inode");
    break;}}
  return 1;
}

int err10(int fsfd){
  uchar buf[BSIZE];
	rsect(SUPERBLOCK,buf);
	memmove(&sb, buf, sizeof(sb));

  int inum;
  struct dinode current_inode;
  for (inum=0; inum<sb.ninodes; inum++){	
  if (lseek(fsfd, sb.inodestart * BSIZE + inum * sizeof(struct dinode), SEEK_SET) != sb.inodestart * BSIZE + inum * sizeof(struct dinode)){  //move to correct location
	  perror("lseek");
    exit(1);
	}
	if(read(fsfd, buf, sizeof(struct dinode))!=sizeof(struct dinode)){	//read inode info into buffer
	  perror("read");
    exit(1);
	}
  memmove(&current_inode, buf, sizeof(current_inode));
  if (current_inode.type==2){
    current_inode.type=0;
    write(fsfd,&current_inode,sizeof(current_inode));
    printf("marked a used inode as free inode\n");
    break;}}
  return 1;
}

int err12(int fsfd){
    uchar buf[BSIZE];
    rsect(SUPERBLOCK,buf);
    memmove(&sb, buf, sizeof(sb));

    int inum = 1;
    struct dinode current_inode;

    if (lseek(fsfd, sb.inodestart * BSIZE + inum * sizeof(struct dinode), SEEK_SET) != sb.inodestart * BSIZE + inum * sizeof(struct dinode)){  //move to correct location
        perror("lseek");
        exit(1);
    }
	if(read(fsfd, buf, sizeof(struct dinode))!=sizeof(struct dinode)){	//read inode info into buffer
	    perror("read");
        exit(1);
	}

    memmove(&current_inode, buf, sizeof(current_inode));
    current_inode.type = T_DIR;
    current_inode.nlink = T_ERR;
    write(fsfd,&current_inode,sizeof(current_inode));

    printf("Changed the type of an inode to directory inode and changed the number of links to greater than 1");

    return 1;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////


//////Switch Table////////

// int switch_table(int errno){
//     if(errno == 1){
//         err1(fsfd);
//     }
//     else if(errno = 2){
//         err2(fsfd);
//     }
//     else if(errno = 3){
//         err3(fsfd);
//     }
//     else if(errno = 4){
//         err4(fsfd);
//     }
//     else if(errno = 5){
//         err5(fsfd);
//     }
//     else if(errno = 6){
//         err6(fsfd);
//     }
//     else if(errno = 7){
//         err7(fsfd);
//     }
//     else if(errno = 8){
//         err8(fsfd);
//     }
//     else if(errno = 9){
//         err9(fsfd);
//     }
//     else if(errno = 10){
//         err10(fsfd);
//     }
//     else if(errno = 11){
//         err11(fsfd);
//     }
//     else if(errno = 12){
//         err12(fsfd);
//     }
//     else{
//     printf("ERROR NO. should be int between (1-12)");
//     }
//     return 0;
// }
/////////////////////////

int main(int argc, char* argv[]){

    if(argc != 3){
        printf("Usage : ./error.o (ERROR No.{1-12}) (File System .img file)\n");
        return 0;
    }    

    // Read the File System image with file discriptor fsfd
    if ((fsfd = open (argv[2],O_RDWR, 0666)) == -1) { /* open/validate file */
        perror ("Error opening file :\n");
        exit (EXIT_FAILURE);
    }

    diskinfo(fsfd);
    //switch_table(argv[1]);
    return 0;
}
