#include <stdint.h>
#include <string.h>
#include "stm32l1xx_hal_flash.h"
#include "utilities.h"
#include "partition.h"
#include "app.h"


#define FLASH_BASE_ADDR  0x0801E000


#define MAX_ARGV    64

struct partition_map_ {
    //partition_index part_index;
    uint16_t part_size;
    uint32_t part_addr;   
};

//static const struct partition_map_ partition_map[] = {
//     512,   FLASH_BASE_ADDR, //PARTITION_0,
//     1024,  FLASH_BASE_ADDR+512, //TABLE_0_0,
//     1024,  FLASH_BASE_ADDR+512+1024, //TABLE_0_1,
//     512,   FLASH_BASE_ADDR+512+1024+1024, //PARTITION_1,
//     1024,  FLASH_BASE_ADDR+512+1024+1024+512, //TABLE_1_0,
//     1024,  FLASH_BASE_ADDR+512+1024+1024+512+1024, //
//};

static const struct partition_map_ partition_map[] = {
     512,   FLASH_BASE_ADDR, //PARTITION_0,
     2048,  FLASH_BASE_ADDR+512, //TABLE_0_0,
     2048,  FLASH_BASE_ADDR+512+2048, //TABLE_0_1,
     256,   FLASH_BASE_ADDR+512+2048+2048, //PARTITION_1,
     256,  FLASH_BASE_ADDR+512+2048+2048+256, //TABLE_1_0,
     256,  FLASH_BASE_ADDR+512+2048+2048+256+256, //
};

//static const struct partition_map_ partition_map[] = {
//     32,   FLASH_BASE_ADDR, //PARTITION_0,
//     2048,  FLASH_BASE_ADDR+32, //TABLE_0_0,
//     2048,  FLASH_BASE_ADDR+32+2048, //TABLE_0_1,
//     32,   FLASH_BASE_ADDR+32+2048+2048, //PARTITION_1,
//     32,  FLASH_BASE_ADDR+32+2048+2048+32, //TABLE_1_0,
//     32,  FLASH_BASE_ADDR+32+2048+2048+32+32, //
//};

static void _ENABLE_FLASH(void)
{
    HAL_FLASH_Unlock(); 
}

static void _DISABLE_FLASH(void)
{
    HAL_FLASH_Lock(); 
}

static void read_data(uint32_t addr, void *buffer, uint16_t len)
{
    //(void)test_table;
    
    memcpy(buffer, (void *)addr, len);
}

 void write_data(uint32_t wr_addr, void *buffer, uint16_t wr_len)
{
    int i;
    uint32_t PAGEError = 0;
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t *wr_data = buffer;
    
    if (wr_addr % 4 != 0 || wr_len%4 != 0) {
        while(1);
    }  
   
    _ENABLE_FLASH();

    for (i = 0; i < wr_len/4; i++) {
        if (wr_addr % FLASH_PAGE_SIZE == 0) {
            
            EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
            EraseInitStruct.PageAddress = wr_addr;
            EraseInitStruct.NbPages     = 1;
            HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);
        }
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, wr_addr, *wr_data);
        wr_data++;
        wr_addr += 4;
    }
    _DISABLE_FLASH();
}

 uint32_t get_active_partition(partition_index index)
{
    uint32_t key_word;
    uint32_t addr = partition_map[index].part_addr;
    
    read_data(addr, (char *)&key_word, sizeof(key_word));
    
    if (key_word == PARTITION_MAGIC_WORD) {
        return partition_map[index+2].part_addr;
    } else {
        return partition_map[index+1].part_addr;
    }
}

 uint32_t get_passive_partition(partition_index index)
{
    uint32_t key_word;
    uint32_t addr = partition_map[index].part_addr;
    
    read_data(addr, (char *)&key_word, sizeof(key_word));
    
    if (key_word == PARTITION_MAGIC_WORD) {
        return partition_map[index+1].part_addr;
    } else {
        return partition_map[index+2].part_addr;
    }
}

 void set_active_partition(partition_index index)
{
    uint32_t key_word;
    uint32_t addr = partition_map[index].part_addr;
    
    read_data(addr, (char *)&key_word, sizeof(key_word));
    
    if (key_word == PARTITION_MAGIC_WORD) {   
        key_word = 0XFFFFFFFF;
        write_data(addr, &key_word, sizeof(key_word));
    } else {
        key_word = PARTITION_MAGIC_WORD;
        write_data(addr, &key_word, sizeof(key_word));
    }
}



int read_partition(partition_index partition, char *out, uint16_t out_len)
{
    uint16_t crc;
    table_header_t table;
    uint32_t base_addr;

    base_addr = get_active_partition(partition);
    read_data(base_addr, &table, sizeof(table_header_t));
    p_log("R- base_addr:0x%x ,crc:0x%x\r\n", base_addr, table.crc);
    
    if (table.magic != PARTITION_MAGIC_WORD) {
        p_log("Table magic error! base_addr:0x%x\r\n", base_addr); 
        return -1;
    }
    if (table.length != out_len) {
        p_log("Table length error! base_addr:0x%x\r\n", base_addr);
        return -1;    
    }
 
    read_data(base_addr + sizeof(table_header_t), out, table.length);
    crc = crc_calc(0, out, out + table.length);
    if (crc != table.crc) {
        p_log("Table crc error! base_addr:0x%x , crc:0x%x\r\n", base_addr, crc);
        return -1;       
    }

    return 0;
}

int write_partition(partition_index partition, char *in, uint16_t in_len)
{
    uint16_t crc;
    table_header_t table;
    uint32_t base_addr;
    
    base_addr = get_passive_partition(partition);
    
    table.magic = PARTITION_MAGIC_WORD;
    table.length = in_len;
    crc = crc_calc(0, in, in + in_len);
    table.crc = crc;
	
    p_log("W- base_addr:0x%x, crc:0x%x\r\n", base_addr, crc);
	
    write_data(base_addr, &table, sizeof(table_header_t));
    
    write_data(base_addr+sizeof(table_header_t), in, in_len);

    set_active_partition(partition);
    
    return 0;
}



