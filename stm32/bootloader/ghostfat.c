//  Functions for flashing to ROM and writing UF2 files.
#include <string.h>
#include <logger/logger.h>
#include <baseloader/baseloader.h>
#include "uf2.h"
#include "target.h"

typedef struct {
    uint8_t JumpInstruction[3];
    uint8_t OEMInfo[8];
    uint16_t SectorSize;
    uint8_t SectorsPerCluster;
    uint16_t ReservedSectors;
    uint8_t FATCopies;
    uint16_t RootDirectoryEntries;
    uint16_t TotalSectors16;
    uint8_t MediaDescriptor;
    uint16_t SectorsPerFAT;
    uint16_t SectorsPerTrack;
    uint16_t Heads;
    uint32_t HiddenSectors;
    uint32_t TotalSectors32;
    uint8_t PhysicalDriveNum;
    uint8_t Reserved;
    uint8_t ExtendedBootSig;
    uint32_t VolumeSerialNumber;
    uint8_t VolumeLabel[11];
    uint8_t FilesystemIdentifier[8];
} __attribute__((packed)) FAT_BootBlock;

typedef struct {
    char name[8];
    char ext[3];
    uint8_t attrs;
    uint8_t reserved;
    uint8_t createTimeFine;
    uint16_t createTime;
    uint16_t createDate;
    uint16_t lastAccessDate;
    uint16_t highStartCluster;
    uint16_t updateTime;
    uint16_t updateDate;
    uint16_t startCluster;
    uint32_t size;
} __attribute__((packed)) DirEntry;

static size_t flashSize(void) {
    return FLASH_SIZE_OVERRIDE;
}

// #define DBG NOOP
// #define DBG DMESG
#define DBG(a, b) {} 

struct TextFile {
    const char name[11];
    const char *content;
};

#define NUM_FAT_BLOCKS UF2_NUM_BLOCKS

#define STR0(x) #x
#define STR(x) STR0(x)
const char infoUf2File[] = UF2_INFO_TEXT;

const char indexFile[] = //
    "<!doctype html>\n"
    "<html>"
    "<body>"
    "<script>\n"
    "location.replace(\"" INDEX_URL "\");\n"
    "</script>"
    "</body>"
    "</html>\n";

static const struct TextFile info[] = {
    {.name = "INFO_UF2TXT", .content = infoUf2File},
    {.name = "INDEX   HTM", .content = indexFile},
    {.name = "CURRENT UF2"},
};
#define NUM_INFO (int)(sizeof(info) / sizeof(info[0]))

#define UF2_SIZE (flashSize() * 2)
#define UF2_SECTORS (UF2_SIZE / 512)
#define UF2_FIRST_SECTOR (NUM_INFO + 1)
#define UF2_LAST_SECTOR (uint32_t)(UF2_FIRST_SECTOR + UF2_SECTORS - 1)

#define RESERVED_SECTORS 1
#define ROOT_DIR_SECTORS 4
#define SECTORS_PER_FAT ((NUM_FAT_BLOCKS * 2 + 511) / 512)

#define START_FAT0 RESERVED_SECTORS
#define START_FAT1 (START_FAT0 + SECTORS_PER_FAT)
#define START_ROOTDIR (START_FAT1 + SECTORS_PER_FAT)
#define START_CLUSTERS (START_ROOTDIR + ROOT_DIR_SECTORS)

static const FAT_BootBlock BootBlock = {
    .JumpInstruction = {0xeb, 0x3c, 0x90},
    .OEMInfo = "UF2 UF2 ",
    .SectorSize = 512,
    .SectorsPerCluster = 1,
    .ReservedSectors = RESERVED_SECTORS,
    .FATCopies = 2,
    .RootDirectoryEntries = (ROOT_DIR_SECTORS * 512 / 32),
    .TotalSectors16 = NUM_FAT_BLOCKS - 2,
    .MediaDescriptor = 0xF8,
    .SectorsPerFAT = SECTORS_PER_FAT,
    .SectorsPerTrack = 1,
    .Heads = 1,
    .ExtendedBootSig = 0x29,
    .VolumeSerialNumber = 0x00420042,
    .VolumeLabel = VOLUME_LABEL,
    .FilesystemIdentifier = "FAT16   ",
};

#define NO_CACHE 0xffffffff

__attribute__ ((section(".boot_buf")))  //  Place this flash buffer in high memory so it can be reused in Application Mode.
uint8_t flashBuf[FLASH_PAGE_SIZE] __attribute__((aligned(4)));  //  Used by bootloader.c and ghostfat.c.

static uint32_t flashAddr = NO_CACHE;
static bool firstFlush = true;
static bool hadWrite = false;
static uint32_t ms;
static uint32_t resetTime;
static uint32_t lastFlush;

int base_flash_program_array(uint16_t *dest0, const uint16_t *src0, size_t half_word_count0) {
	//  Return the number of half-words flashed.
    base_para.dest = (uint32_t) dest0;
    base_para.src = (uint32_t) src0;
    base_para.byte_count = half_word_count0 * 2;
    base_para.restart = 0;
    base_para.preview = 0;
    baseloader_start();
	int bytes_flashed = base_para.result;
	return (bytes_flashed > 0) ? bytes_flashed / 2 : bytes_flashed;
}

void flash_flush(void) {
    //  Flush the page of cached flashing data to ROM.
    lastFlush = ms;
    if (flashAddr == NO_CACHE) { return; }
    if (firstFlush) { firstFlush = false; }  //  TODO: disable bootloader or something
    DBG("Flush at %x", flashAddr);
    if (memcmp(flashBuf, (void *)flashAddr, FLASH_PAGE_SIZE) != 0) {
        //  If the page contents are different, write to ROM.
        debug_print("-> "); debug_printhex_unsigned((size_t) flashAddr); debug_print(" "); //// DBG("Write flush at %x", flashAddr);
        int words_flashed = base_flash_program_array((void *)flashAddr, (void*)flashBuf, FLASH_PAGE_SIZE / 2);
        if (words_flashed != FLASH_PAGE_SIZE / 2) { debug_print("*** ERROR: Flash failed "); debug_print_int(words_flashed); debug_println(""); debug_force_flush(); }
    }
    flashAddr = NO_CACHE;
}

void flash_write(uint32_t dst, const uint8_t *src, int byte_count) {
    //  Write len bytes from src to ROM at address dst.  The writing is buffered in RAM until flash_flush() is called.    
    //  TODO: Support other memory sizes.
    int valid = 
        (dst >= 0x08000000 && (dst + byte_count) < 0x8010000 &&
        (
            ((uint32_t) src >= 0x08000000 && ((uint32_t) src + byte_count) < 0x08010000) ||
            ((uint32_t) src >= 0x20000000 && ((uint32_t) src + byte_count) < 0x20005000)
        ));
    if (!valid) {
        debug_print("**** ERROR: Invalid flash write, dst "); debug_printhex_unsigned(dst); 
        debug_print(", src "); debug_printhex_unsigned((uint32_t) src);
        debug_print(", len "); debug_printhex_unsigned(byte_count); debug_println(""); debug_force_flush();
        return;
    }
    while (byte_count > 0) {
        //  Copy one page at a time.
        int len = (byte_count > FLASH_PAGE_SIZE) ? FLASH_PAGE_SIZE : byte_count;
        uint32_t newAddr = dst & ~(FLASH_PAGE_SIZE - 1);
        hadWrite = true;
        if (newAddr != flashAddr) {  //  If the cache has a different page...
            flash_flush();           //  Flush the previous cached page.
            flashAddr = newAddr;
            memcpy(flashBuf, (void *)newAddr, FLASH_PAGE_SIZE);  //  Copy the current ROM page into the buffer.
        }
        //  Copy the new contents into the buffer, which may be a part of the buffer.
        uint32_t offset = dst & (FLASH_PAGE_SIZE - 1);
        memcpy(flashBuf + offset, src, len);
        //  Copy next page.
        dst += len;
        src += len;
        byte_count -= len;
    }
}

static void uf2_timer_start(int delay) {
    resetTime = ms + delay;
}

void ghostfat_1ms() {
    // called roughly every 1ms
    ms++;
    if (resetTime && ms >= resetTime) {
        debug_println("ghostfat_1ms boot_target_manifest_app");  debug_flush();  ////
        flash_flush();
        boot_target_manifest_app();
        while (1);
    }
    if (lastFlush && ms - lastFlush > 100) {
        flash_flush();
    }
}

static void padded_memcpy(char *dst, const char *src, int len) {
    for (int i = 0; i < len; ++i) {
        if (*src)
            *dst = *src++;
        else
            *dst = ' ';
        dst++;
    }
}

int read_block(uint32_t block_no, uint8_t *data) {
    memset(data, 0, 512);
    uint32_t sectionIdx = block_no;

    if (block_no == 0) {
        memcpy(data, &BootBlock, sizeof(BootBlock));
        data[510] = 0x55;
        data[511] = 0xaa;
        // logval("data[0]", data[0]);
    } else if (block_no < START_ROOTDIR) {
        sectionIdx -= START_FAT0;
        // logval("sidx", sectionIdx);
        if (sectionIdx >= SECTORS_PER_FAT)
            sectionIdx -= SECTORS_PER_FAT;
        if (sectionIdx == 0) {
            data[0] = 0xf0;
            for (int i = 1; i < NUM_INFO * 2 + 4; ++i) {
                data[i] = 0xff;
            }
        }
        for (int i = 0; i < 256; ++i) {
            uint32_t v = sectionIdx * 256 + i;
            if (UF2_FIRST_SECTOR <= v && v <= UF2_LAST_SECTOR)
                ((uint16_t *)(void *)data)[i] = v == UF2_LAST_SECTOR ? 0xffff : v + 1;
        }
    } else if (block_no < START_CLUSTERS) {
        sectionIdx -= START_ROOTDIR;
        if (sectionIdx == 0) {
            DirEntry *d = (void *)data;
            padded_memcpy(d->name, (const char *)BootBlock.VolumeLabel, 11);
            d->attrs = 0x28;
            for (int i = 0; i < NUM_INFO; ++i) {
                d++;
                const struct TextFile *inf = &info[i];
                d->size = inf->content ? strlen(inf->content) : UF2_SIZE;
                d->startCluster = i + 2;
                padded_memcpy(d->name, inf->name, 11);
            }
        }
    } else {
        sectionIdx -= START_CLUSTERS;
        if (sectionIdx < NUM_INFO - 1) {
            memcpy(data, info[sectionIdx].content, strlen(info[sectionIdx].content));
        } else {
            sectionIdx -= NUM_INFO - 1;
            uint32_t addr = sectionIdx * 256;
            if (addr < flashSize()) {
                UF2_Block *bl = (void *)data;
                bl->magicStart0 = UF2_MAGIC_START0;
                bl->magicStart1 = UF2_MAGIC_START1;
                bl->magicEnd = UF2_MAGIC_END;
                bl->blockNo = sectionIdx;
                bl->numBlocks = flashSize() / 256;
                bl->targetAddr = addr | 0x8000000;
                bl->payloadSize = 256;
                memcpy(bl->data, (void *)addr, bl->payloadSize);
            }
        }
    }

    return 0;
}

static void write_block_core(uint32_t block_no, const uint8_t *data, bool quiet, WriteState *state) {
    const UF2_Block *bl = (const void *)data;

    (void)block_no;

    // DBG("Write magic: %x", bl->magicStart0);

    if (!is_uf2_block(bl) || !UF2_IS_MY_FAMILY(bl)) {
        return;
    }

    if ((bl->flags & UF2_FLAG_NOFLASH) || bl->payloadSize > 256 || (bl->targetAddr & 0xff) ||
        bl->targetAddr < USER_FLASH_START || bl->targetAddr + bl->payloadSize > USER_FLASH_END) {
        debug_print("write_block_core skip "); debug_print_unsigned((size_t) bl->targetAddr); debug_println(""); debug_flush();
        DBG("Skip block at %x", bl->targetAddr);
        // this happens when we're trying to re-flash CURRENT.UF2 file previously
        // copied from a device; we still want to count these blocks to reset properly
    } else {
        // logval("write block at", bl->targetAddr);
        debug_print("write_block_core "); debug_print_unsigned((size_t) bl->targetAddr); debug_println(""); debug_flush();
        DBG("Write block at %x", bl->targetAddr);
        flash_write(bl->targetAddr, bl->data, bl->payloadSize);
    }

    bool isSet = false;

    if (state && bl->numBlocks) {
        if (state->numBlocks != bl->numBlocks) {
            if (bl->numBlocks >= MAX_BLOCKS || state->numBlocks)
                state->numBlocks = 0xffffffff;
            else
                state->numBlocks = bl->numBlocks;
        }
        if (bl->blockNo < MAX_BLOCKS) {
            uint8_t mask = 1 << (bl->blockNo % 8);
            uint32_t pos = bl->blockNo / 8;
            if (!(state->writtenMask[pos] & mask)) {
                // logval("incr", state->numWritten);
                state->writtenMask[pos] |= mask;
                state->numWritten++;
            }
            if (state->numWritten >= state->numBlocks) {
                // wait a little bit before resetting, to avoid Windows transmit error
                // https://github.com/Microsoft/uf2-samd21/issues/11
                if (!quiet) {
                    uf2_timer_start(30);
                    isSet = true;
                }
            }
        }
        //DBG("wr %d=%d (of %d)", state->numWritten, bl->blockNo, bl->numBlocks);
    }

    if (!isSet && !quiet) {
        uf2_timer_start(500);
    }
}


WriteState wrState;

int write_block(uint32_t lba, const uint8_t *copy_from)
{
    write_block_core(lba, copy_from, false, &wrState);
    return 0;
}
