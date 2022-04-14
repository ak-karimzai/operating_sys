#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h> 
#define PAGE_SIZE 0x1000

#define FIND_LIB_NAME

#include "pagemap.h"

static void print_page(uint64_t address, uint64_t data,
    const char *lib_name, FILE *out) {

    fprintf(out, "0x%-16lx : pfn %-16lx soft-dirty %ld file/shared %ld "
        "swapped %ld present %ld library %s\n",
        address,
        data & 0x7fffffffffffff,
        (data >> 55) & 1,
        (data >> 61) & 1,
        (data >> 62) & 1,
        (data >> 63) & 1,
        lib_name);
}

static void handle_virtual_range(int pagemap, uint64_t start_address,
    uint64_t end_address, const char *lib_name, FILE *out) {

    for(uint64_t i = start_address; i < end_address; i += 0x1000) {
        uint64_t data;
        uint64_t index = (i / PAGE_SIZE) * sizeof(data);
        if(pread(pagemap, &data, sizeof(data), index) != sizeof(data)) {
            if(errno) perror("pread");
            break;
        }

        print_page(i, data, lib_name, out);
    }
}

static void parse_maps(const char *maps_file, const char *pagemap_file, FILE *out) {
    int maps = open(maps_file, O_RDONLY);
    if(maps < 0) return;

    int pagemap = open(pagemap_file, O_RDONLY, out);
    if(pagemap < 0) {
        close(maps);
        return;
    }

    char buffer[BUFSIZ];
    int offset = 0;

    for(;;) {
        ssize_t length = read(maps, buffer + offset, sizeof buffer - offset);
        if(length <= 0) break;

        length += offset;

        for(size_t i = offset; i < (size_t)length; i ++) {
            uint64_t low = 0, high = 0;
            if(buffer[i] == '\n' && i) {
                size_t x = i - 1;
                while(x && buffer[x] != '\n') x --;
                if(buffer[x] == '\n') x ++;
                size_t beginning = x;

                while(buffer[x] != '-' && x+1 < sizeof buffer) {
                    char c = buffer[x ++];
                    low *= 16;
                    if(c >= '0' && c <= '9') {
                        low += c - '0';
                    }
                    else if(c >= 'a' && c <= 'f') {
                        low += c - 'a' + 10;
                    }
                    else break;
                }

                while(buffer[x] != '-' && x+1 < sizeof buffer) x ++;
                if(buffer[x] == '-') x ++;

                while(buffer[x] != ' ' && x+1 < sizeof buffer) {
                    char c = buffer[x ++];
                    high *= 16;
                    if(c >= '0' && c <= '9') {
                        high += c - '0';
                    }
                    else if(c >= 'a' && c <= 'f') {
                        high += c - 'a' + 10;
                    }
                    else break;
                }

                const char *lib_name = 0;
#ifdef FIND_LIB_NAME
                for(int field = 0; field < 4; field ++) {
                    x ++;  // skip space
                    while(buffer[x] != ' ' && x+1 < sizeof buffer) x ++;
                }
                while(buffer[x] == ' ' && x+1 < sizeof buffer) x ++;

                size_t y = x;
                while(buffer[y] != '\n' && y+1 < sizeof buffer) y ++;
                buffer[y] = 0;

                lib_name = buffer + x;
#endif

                handle_virtual_range(pagemap, low, high, lib_name, out);

#ifdef FIND_LIB_NAME
                buffer[y] = '\n';
#endif
            }
        }
    }

    close(maps);
    close(pagemap);
}


void pagemap_info(const char *proc_path, FILE *out) {
    char maps_file[BUFSIZ];
    char pagemap_file[BUFSIZ];

    snprintf(maps_file, sizeof(maps_file),
        "%s/maps", proc_path);
    snprintf(pagemap_file, sizeof(pagemap_file),
        "%s/pagemap", proc_path);

    parse_maps(maps_file, pagemap_file, out);
}