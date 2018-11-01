#ifndef _PARTITION_H_
#define _PARTITION_H_


#define PARTITION_MAGIC_WORD (('R' << 0)|('L' << 8)|('T' << 16)|('H' << 24))

typedef struct {
    uint32_t magic;
    uint32_t length;
    uint32_t crc;
}table_header_t;


typedef enum  {
    PARTITION_0 = 0,
    TABLE_0_0,
    TABLE_0_1,
    PARTITION_1,
    TABLE_1_0,
    TABLE_1_1,
}partition_index;


#define p_log(...)    // e_printf("part:"##__VA_ARGS__)


int read_partition(partition_index partition, char *out, uint16_t out_len);


#endif
