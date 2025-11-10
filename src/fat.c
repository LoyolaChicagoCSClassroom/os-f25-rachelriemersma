#include "fat.h"
#include "ide.h"
#include "rprintf.h"
#include <stddef.h>

#define SECTOR_SIZE 512

// Global variables
struct boot_sector *bs;
char boot_sector_buf[SECTOR_SIZE];
char fat_table[8 * SECTOR_SIZE];
char rde_region[16384];  // Root directory entries
unsigned int root_sector;
unsigned int data_region_start;

extern void putc(int data);

// Initialize FAT filesystem
int fatInit(void) {
    // Read boot sector (sector 0)
    ata_lba_read(0, (unsigned char *)boot_sector_buf, 1);
    bs = (struct boot_sector *)boot_sector_buf;
    
    // Validate boot signature
    if (bs->boot_signature != 0xAA55) {
        esp_printf(putc, "Invalid boot signature: 0x%x\r\n", bs->boot_signature);
        return -1;
    }
    
    // Print filesystem info
    esp_printf(putc, "FAT filesystem detected\r\n");
    esp_printf(putc, "Bytes per sector: %d\r\n", bs->bytes_per_sector);
    esp_printf(putc, "Sectors per cluster: %d\r\n", bs->num_sectors_per_cluster);
    
    // Calculate root directory sector
    root_sector = bs->num_reserved_sectors + 
                  (bs->num_fat_tables * bs->num_sectors_per_fat) +
                  bs->num_hidden_sectors;
    
    // Calculate data region start
    data_region_start = root_sector + 
                       ((bs->num_root_dir_entries * 32) / bs->bytes_per_sector);
    
    // Read FAT table
    ata_lba_read(bs->num_reserved_sectors, (unsigned char *)fat_table, 
                 bs->num_sectors_per_fat);
    
    // Read root directory entries
    ata_lba_read(root_sector, (unsigned char *)rde_region, 32);
    
    return 0;
}

// Helper: Extract filename from RDE
void extract_filename(struct root_directory_entry *rde, char *fname) {
    int k = 0;
    
    // Copy filename (up to 8 chars)
    while (rde->file_name[k] != ' ' && k < 8) {
        fname[k] = rde->file_name[k];
        k++;
    }
    
    // Add extension if present
    if (rde->file_extension[0] != ' ') {
        fname[k++] = '.';
        int n = 0;
        while (rde->file_extension[n] != ' ' && n < 3) {
            fname[k++] = rde->file_extension[n];
            n++;
        }
    }
    
    fname[k] = '\0';
}

// Helper: Compare strings
int strcmp_simple(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

// Open a file and return its RDE
struct file *fatOpen(const char *filename) {
    struct root_directory_entry *rde_tbl = (struct root_directory_entry *)rde_region;
    
    // Search through root directory entries
    for (int i = 0; i < bs->num_root_dir_entries; i++) {
        char temp_name[16];
        
        // Skip deleted or empty entries
        if (rde_tbl[i].file_name[0] == 0x00 || rde_tbl[i].file_name[0] == 0xE5) {
            continue;
        }
        
        // Extract filename
        extract_filename(&rde_tbl[i], temp_name);
        
        // Compare with requested filename
        if (strcmp_simple(temp_name, filename) == 0) {
            // Found the file!
            static struct file f;
            f.rde = rde_tbl[i];
            f.start_cluster = rde_tbl[i].cluster;
            return &f;
        }
    }
    
    return NULL;  // File not found
}

// Read file contents into buffer
int fatRead(struct file *file, void *buffer, unsigned int size) {
    if (file == NULL) {
        return -1;
    }
    
    unsigned int cluster = file->start_cluster;
    unsigned int bytes_read = 0;
    unsigned int bytes_to_read = (size < file->rde.file_size) ? size : file->rde.file_size;
    
    char cluster_buf[CLUSTER_SIZE];
    
    while (bytes_read < bytes_to_read && cluster < 0xFF8) {
        // Calculate sector number for this cluster
        unsigned int sector = data_region_start + ((cluster - 2) * bs->num_sectors_per_cluster);
        
        // Read cluster
        ata_lba_read(sector, (unsigned char *)cluster_buf, bs->num_sectors_per_cluster);
        
        // Copy data to buffer
        unsigned int bytes_in_cluster = CLUSTER_SIZE;
        if (bytes_read + bytes_in_cluster > bytes_to_read) {
            bytes_in_cluster = bytes_to_read - bytes_read;
        }
        
        for (unsigned int i = 0; i < bytes_in_cluster; i++) {
            ((char *)buffer)[bytes_read++] = cluster_buf[i];
        }
        
        // Get next cluster from FAT
        cluster = ((uint16_t *)fat_table)[cluster];
    }
    
    return bytes_read;
}