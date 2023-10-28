#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#define __DEBUG
#ifdef __DEBUG
#define DEBUG(format,...) printf(""format"", ##__VA_ARGS__)  
#else
#define DEBUG(...)
#endif

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#define  ATTR_READ_ONLY (0x01)
#define ATTR_HIDDEN (0x02) 
#define ATTR_SYSTEM (0x04)
#define ATTR_VOLUME_ID (0x08)
#define ATTR_DIRECTORY (0x10)
#define ATTR_ARCHIVE (0x20)
#define ATTR_LONG_NAME  (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)

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
typedef struct _longnameentry {
  u8  LDIR_Ord;
  u16  LDIR_Name1[5];
  u8  LDIR_Attr;
  u8  LDIR_Type;
  u8  LDIR_Chksum;
  u16  LDIR_Name2[6];
  u16 LDIR_FstClusLO;
  u16 LDIR_Name3[2];
} __attribute__((packed)) longnameentry;

void *map_disk(const char *fname);
void travel_data(uint8_t * data_start); 
static int cluster_sz;
static int BPB_SecPerClus;
static int data_cluster_count;
static int BPB_RootClus;
int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s fs-image\n", argv[0]);
    exit(1);
  }

  setbuf(stdout, NULL);

  assert(sizeof(struct fat32hdr) == 512); // defensive
  assert(sizeof(direntry) == 32);
  assert(sizeof(longnameentry) == 32);
  // map disk image to memory
  struct fat32hdr *hdr = map_disk(argv[1]);
  assert(hdr->BS_FilSysType[3] == '3' && hdr->BS_FilSysType[4] == '2');

  // TODO: frecov
  DEBUG("bytes per sec:%d\n", hdr->BPB_BytsPerSec);
  DEBUG("Reserved sec %d\n", hdr->BPB_RsvdSecCnt);
  DEBUG("Fat num:%d\n", hdr->BPB_NumFATs);
  DEBUG("Fat sec:%d\n", hdr->BPB_FATSz32);
  DEBUG("total sect:%d\n", hdr->BPB_TotSec32);
  DEBUG("root clus:%d\n", hdr->BPB_RootClus);
  // TODO: computer the start address of dir
  int special_sec = hdr->BPB_RsvdSecCnt + hdr->BPB_FATSz32 * hdr->BPB_NumFATs;
  int data_sec = hdr->BPB_TotSec32 -special_sec;
  data_cluster_count = data_sec / hdr->BPB_SecPerClus;
  DEBUG("data sec count:%d, cluster:%d\n", data_sec, data_cluster_count);
  BPB_SecPerClus = hdr->BPB_SecPerClus;
  cluster_sz = hdr->BPB_BytsPerSec * BPB_SecPerClus;
  DEBUG("cluster size:%d\n", cluster_sz);
  BPB_RootClus = hdr->BPB_RootClus;
  travel_data((uint8_t*)(hdr + special_sec));
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
static int is_dir_valid(direntry * dir) {
  return dir->DIR__Name[0] != 0x00 && dir->DIR__Name[0] != 0xe5;
}
static char * parse_entry_name(direntry * dir, int long_name_count) {
  if(long_name_count == 0) {
    // for(int i = 0; i < 11; i ++) {
    //   DEBUG("%c", dir->DIR__Name[i]);
    // }
    // DEBUG("\n");
    return NULL;
  } else {
    int name_size = long_name_count * 13;
    char * name = malloc(name_size + 1);
    int name_index = 0;
    for(int i = 0; i < long_name_count; i ++) {
      longnameentry *name_entry = (longnameentry *)(dir - i - 1);
      for(int i = 0; i < 5; i ++)
        name[name_index  ++] = (char) name_entry->LDIR_Name1[i];
      for(int i = 0; i < 6; i ++)
        name[name_index  ++] = (char) name_entry->LDIR_Name2[i];
      for(int i = 0; i < 2; i ++)
        name[name_index  ++] = (char) name_entry->LDIR_Name3[i];
    }
    name[name_size] = '\0';
    return name;
  }
}
// TODO: pass file size and generate file
int handle_file(char * name, int N, uint8_t * first_data_sect, int size) {
  if(N < 2 || N - 2 > data_cluster_count)
    return 0;;
  uint8_t * first = first_data_sect + (N - 2) * cluster_sz;
  if(first[0] == 'B' && first[1] == 'M') {
    DEBUG("It is bmp head cluster\n");
    FILE *f = fopen(name, "w");
    for(int i = 0; i < size; i ++) {
      fwrite(first + i, 1, 1, f);
    }
    fflush(f);
    fclose(f);
    return 1;
  }
  return 0;
}
// TODO: find other dirs
void travel_data(uint8_t * first_data_sect) {
  int sum = 0;
  direntry* root_dir = (direntry*)(first_data_sect + cluster_sz * (BPB_RootClus - 1));
  int long_name_count = 0;
  for(int i = 0; i < cluster_sz / 32; i ++) {
    direntry * dir = root_dir + i;
    if(!is_dir_valid(dir))
      continue;
    DEBUG("attr:%x\n", dir->DIR_Attr);
    if(dir->DIR_Attr == ATTR_DIRECTORY) {
      long_name_count = 0;
      DEBUG("Is sub dir\n");
    } else if(dir->DIR_Attr == ATTR_LONG_NAME) {
      long_name_count ++;
      // DEBUG("Is long name %d\n", long_name_count);
    } else {
      char * name = parse_entry_name(dir, long_name_count);
      DEBUG("%s\n", name);
      long_name_count = 0;
      int bmp_cluster = dir->DIR_FstClusHI << 16; 
      bmp_cluster += dir->DIR_FstClusLO;
      DEBUG("bmp cluster: %d bmp size: %d\n", bmp_cluster, dir->DIR_FileSize);
      if(handle_file(name, bmp_cluster, first_data_sect, dir->DIR_FileSize))
        sum ++;
      free(name);
    }
 }
  // for(int i = 0; i < data_cluster_count; i ++) {
  //   uint8_t * cur = first_data_sect + i * cluster_sz;
  //   if(cur[0] == 'B' && cur[1] == 'M') {
  //     int cluster_index = i + 2;
  //     DEBUG("%d is bmp head cluster, sec:%d\n", cluster_index, cluster_index * BPB_SecPerClus);
  //     sum ++;
  //   }
  // }
  DEBUG("bmp head cluster is %d\n", sum);
}
