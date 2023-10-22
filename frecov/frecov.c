#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#define  ATTR_READ_ONLY (0x01)
#define ATTR_HIDDEN (0x02) 
#define ATTR_SYSTEM (0x04)
#define ATTR_VOLUME_ID (0x08)
#define ATTR_DIRECTORY (0x10)
#define ATTR_ARCHIVE (0x20)
// Copied from the manual
struct fat32hdr {
  u8  BS_jmpBoot[3];
  u8  BS_OEMName[8];
  u16 BPB_BytsPerSec;
  u8  BPB_SecPerClus;
  u16 BPB_RsvdSecCnt;
  u8  BPB_NumFATs;
  u16 BPB_RootEntCnt;
  u16 BPB_TotSec16;
  u8  BPB_Media;
  u16 BPB_FATSz16;
  u16 BPB_SecPerTrk;
  u16 BPB_NumHeads;
  u32 BPB_HiddSec;
  u32 BPB_TotSec32;
  u32 BPB_FATSz32;
  u16 BPB_ExtFlags;
  u16 BPB_FSVer;
  u32 BPB_RootClus;
  u16 BPB_FSInfo;
  u16 BPB_BkBootSec;
  u8  BPB_Reserved[12];
  u8  BS_DrvNum;
  u8  BS_Reserved1;
  u8  BS_BootSig;
  u32 BS_VolID;
  u8  BS_VolLab[11];
  u8  BS_FilSysType[8];
  u8  __padding_1[420];
  u16 Signature_word;
} __attribute__((packed));

typedef struct _direntry {
  u8  DIR__Name[11];
  u8  DIR_Attr;
  u8  DIR_NTRes;
  u8  DIR_CrtTimeTenth;
  u16 DIR_CrtTime;
  u16 DIR_CrtDate;
  u16 DIR_LstAccDate;
  u16 DIR_FstClusHI;
  u16 DIR_WrtTime;
  u16 DIR_WrtDate;
  u16 DIR_FstClusLO;
  u32 DIR_FileSize;
} __attribute__((packed)) direntry;

void *map_disk(const char *fname);
void travel_data(uint8_t * data_start, int cluster_count, int cluster_sz, int root_cluster); 

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s fs-image\n", argv[0]);
    exit(1);
  }

  setbuf(stdout, NULL);

  assert(sizeof(struct fat32hdr) == 512); // defensive
  assert(sizeof(direntry) == 32);
  // map disk image to memory
  struct fat32hdr *hdr = map_disk(argv[1]);
  assert(hdr->BS_FilSysType[3] == '3' && hdr->BS_FilSysType[4] == '2');

  // TODO: frecov
  printf("bytes per sec:%d\n", hdr->BPB_BytsPerSec);
  printf("Reserved sec %d\n", hdr->BPB_RsvdSecCnt);
  printf("Fat num:%d\n", hdr->BPB_NumFATs);
  printf("Fat sec:%d\n", hdr->BPB_FATSz32);
  printf("total sect:%d\n", hdr->BPB_TotSec32);
  printf("root clus:%d\n", hdr->BPB_RootClus);
  // TODO: computer the start address of dir
  int special_sec = hdr->BPB_RsvdSecCnt + hdr->BPB_FATSz32 * hdr->BPB_NumFATs;
  int data_sec = hdr->BPB_TotSec32 -special_sec;
  int data_cluster = data_sec / hdr->BPB_SecPerClus;
  printf("data sec count:%d, cluster:%d\n", data_sec, data_cluster);
  int cluster_sz = hdr->BPB_BytsPerSec * hdr->BPB_SecPerClus;
  printf("cluster size:%d\n", cluster_sz);
  travel_data((uint8_t*)(hdr + special_sec), data_cluster, cluster_sz, hdr->BPB_RootClus);
  // file system traversal
  munmap(hdr, hdr->BPB_TotSec32 * hdr->BPB_BytsPerSec);
}

void *map_disk(const char *fname) {
  int fd = open(fname, O_RDWR);

  if (fd < 0) {
    perror(fname);
    goto release;
  }

  off_t size = lseek(fd, 0, SEEK_END);
  if (size == -1) {
    perror(fname);
    goto release;
  }

  struct fat32hdr *hdr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  if (hdr == (void *)-1) {
    goto release;
  }

  close(fd);

  if (hdr->Signature_word != 0xaa55 ||
      hdr->BPB_TotSec32 * hdr->BPB_BytsPerSec != size) {
    fprintf(stderr, "%s: Not a FAT file image\n", fname);
    goto release;
  }
  return hdr;

release:
  if (fd > 0) {
    close(fd);
  }
  exit(1);
}
static void print_dir_name(direntry * dir) {
  for(int i = 0; i < 11; i ++) {
    printf("%c", dir->DIR__Name[i]);
  }
  printf("\n");
}
static int is_dir_valid(direntry * dir) {
  return dir->DIR__Name[0] != 0x00 && dir->DIR__Name[0] != 0xe5;
}
void travel_data(uint8_t * data_start, int cluster_count, int cluster_sz, int root_cluster) {
  direntry* root_dir = (direntry*)(data_start + cluster_sz * (root_cluster - 1));
  for(int i = 0; i < cluster_sz / 32; i ++) {
    direntry * dir = root_dir + i;
    if(!is_dir_valid(dir))
      continue;
    print_dir_name(dir);
    if(dir->DIR_Attr == ATTR_DIRECTORY) {
      printf("Is sub dir\n");
    } else {
      int bmp_cluster = dir->DIR_FstClusHI << 16; 
      bmp_cluster += dir->DIR_FstClusLO;
      printf("bmp cluster: %d bmp size: %d\n", bmp_cluster, dir->DIR_FileSize);
    }
 }
  // int sum = 0;
  // for(int i = 0; i < cluster_count; i ++) {
  //   uint8_t * cur = data_start + i * cluster_sz;
  //   if(cur[0] == 'B' && cur[1] == 'M') {
  //     printf("%d is bmp head cluster\n", i);
  //     sum ++;
  //   }
  // }
  // printf("bmp head cluster is %d\n", sum);
}
