#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <dirent.h>
#include <linux/stat.h>
#include <fcntl.h>
#include <assert.h>

#include "main.h"
#include "pagemap.h"

const char *LINK_TO_SELF = "/proc/self";

void check_path(char *proc_path)
{
    size_t len = strlen(proc_path);

    if (len && proc_path[len - 1] == '/') 
        proc_path[len - 1] = 0; 
}

void cmdline_info(const char *proc_path, FILE *out)
{
    char buf[BUFSIZ] = { 0 }, file_path[256] = { 0 };
    snprintf(file_path, BUFSIZ, "%s/cmdline", proc_path);
    FILE *cmdline = fopen(file_path, "r");
    
    fprintf(out, "\t\t***%s***\n\n", file_path);
    
    size_t len;
    while ((len = fread(buf, 1, BUFSIZ, cmdline)) > 0)
    {
        for (size_t i = 0; i < len; i++)
            if (buf[i] == 0)
                buf[i] = 10;

        if (len) buf[len - 1] = 0;
        fputs(buf, out);
    }
    fclose(cmdline);
}

void cwd_info(const char *proc_path, FILE *out)
{
    char buf[BUFSIZ] = { 0 }, file_path[256] = { 0 };
    ssize_t len;

    snprintf(file_path, 256, "%s/cwd", proc_path);
    
    fprintf(out, "\n\n\t\t***%s***\n\n", file_path);
    if ((len = readlink(file_path, buf, BUFSIZ)) != -1)
        buf[len] = 0;
    fprintf(out, "\n%s\n", buf);
}

void environ_info(const char *proc_path, FILE *out)
{
    char buf[BUFSIZ] = { 0 }, file_path[256] = { 0 };
    snprintf(file_path, BUFSIZ, "%s/environ", proc_path);
    FILE *environ = fopen(file_path, "r");
    
    fprintf(out, "\n\n\t\t***%s***\n\n", file_path);
    
    size_t len;
    while ((len = fread(buf, 1, BUFSIZ, environ)) > 0)
    {
        for (size_t i = 0; i < len; i++)
            if (buf[i] == 0)
                buf[i] = 10;

        if (len) buf[len - 1] = 0;
        fputs(buf, out);
    }
    fclose(environ);
}

void stat_info(const char *proc_path, FILE *out)
{
    char buf[BUFSIZ] = { 0 }, file_path[256] = { 0 };
    snprintf(file_path, BUFSIZ, "%s/stat", proc_path);
    FILE *stat = fopen(file_path, "r");
    
    fprintf(out, "\n\n\t\t***%s***\n\n", file_path);
    
    size_t len;
    while ((len = fread(buf, 1, BUFSIZ, stat)) > 0)
    {
        char *token = strtok(buf, " ");
        for (size_t i = 0; token; i++)
        {
            fprintf(out, WITH_DESCR[i], token);
            token = strtok(NULL, " ");
        }
    }
    fclose(stat);
}

void statm_info(const char *proc_path, FILE *out)
{
    char buf[BUFSIZ] = { 0 }, file_path[256] = { 0 };
    snprintf(file_path, BUFSIZ, "%s/statm", proc_path);
    FILE *stat = fopen(file_path, "r");
    
    fprintf(out, "\n\n\t\t***%s***\n\n", file_path);
    
    size_t len;
    while ((len = fread(buf, 1, BUFSIZ, stat)) > 0)
    {
        char *token = strtok(buf, " ");
        for (size_t i = 0; token; i++)
        {
            fprintf(out, STATM_PATTERNS[i], token);
            token = strtok(NULL, " ");
        }
    }
    fclose(stat);
}

void fd_info(const char *proc_path, FILE *out)
{
    char buf[BUFSIZ] = { 0 }, file_path[256] = { 0 }, path[512] = { 0 };
    DIR *dp;
    snprintf(file_path, BUFSIZ, "%s/fd", proc_path);
    fprintf(out, "\n\n\t\t***%s***\n\n", file_path);

    dp = opendir(file_path);
    struct dirent *dirp;
    while (dirp = readdir(dp))
    {
        if (strcmp(dirp->d_name, ".") != 0 && \
            strcmp(dirp->d_name, "..") != 0)
        {
            sprintf(path, "%s/%s", file_path, dirp->d_name);
            readlink(path, buf, BUFSIZ);
            fprintf(out, "%s -> %s\n", dirp->d_name, buf);
        }
    }
    closedir(dp);
}

void exe_info(const char *proc_path, FILE *out)
{
    char buf[BUFSIZ] = { 0 }, file_path[256] = { 0 };
    ssize_t len;

    snprintf(file_path, 256, "%s/exe", proc_path);
    
    fprintf(out, "\n\n\t\t***%s***\n\n", file_path);
    if ((len = readlink(file_path, buf, BUFSIZ)) != -1)
        buf[len] = 0;
    fprintf(out, "\n%s\n", buf);
}

void root_info(const char *proc_path, FILE *out)
{
    char buf[BUFSIZ] = { 0 }, file_path[256] = { 0 };
    ssize_t len;

    snprintf(file_path, 256, "%s/root", proc_path);
    
    fprintf(out, "\n\n\t\t***%s***\n\n", file_path);
    if ((len = readlink(file_path, buf, BUFSIZ)) != -1)
        buf[len] = 0;
    fprintf(out, "\n%s\n", buf);
}


void maps_info(const char *proc_path, FILE *out)
{
    char buf[BUFSIZ] = { 0 }, file_path[256] = { 0 };
    snprintf(file_path, BUFSIZ, "%s/maps", proc_path);
    FILE *maps = fopen(file_path, "r");
    
    fprintf(out, "\n\n\t\t***%s***\n\n", file_path);
    
    size_t len;
    while ((len = fread(buf, 1, BUFSIZ, maps)) > 0)
    {
        buf[len] = 0;
        fputs(buf, out);
    }
    fclose(maps);
}

void comm_info(const char *proc_path, FILE *out)
{
    char buf[BUFSIZ] = { 0 }, file_path[256] = { 0 };
    snprintf(file_path, BUFSIZ, "%s/comm", proc_path);
    FILE *comm = fopen(file_path, "r");
    
    fprintf(out, "\n\n\t\t***%s***\n\n", file_path);
    
    size_t len = fread(buf, 1, BUFSIZ, comm);
    if (len) buf[len] = 0;
    fputs(buf, out);
    fclose(comm);
}

void task_info(const char *proc_path, FILE *out)
{
    char buf[BUFSIZ] = { 0 }, file_path[256] = { 0 }, path[512] = { 0 };
    DIR *dp;
    snprintf(file_path, BUFSIZ, "%s/task", proc_path);
    fprintf(out, "\n\n\t\t***%s***\n\n", file_path);

    dp = opendir(file_path);
    struct dirent *dirp;
    while (dirp = readdir(dp))
    {
        if (strcmp(dirp->d_name, ".") != 0 && \
            strcmp(dirp->d_name, "..") != 0)
        {
            sprintf(path, "%s/%s", file_path, dirp->d_name);
            readlink(path, buf, BUFSIZ);
            fprintf(out, "%s\n", dirp->d_name);
        }
    }
    closedir(dp);
}

void print_page(uint64_t address, uint64_t data, FILE *out)
{
    fprintf(out, "0x%-16lx : %-16lx %-10ld %-10ld %-10ld %-10ld\n", address,
                 data & (((uint64_t)1 << 55) - 1), (data >> 55) & 1,
                (data >> 61) & 1, (data >> 62) & 1, (data >> 63) & 1);
}

void get_pagemap_info(const char *proc, FILE *out)
{
    fprintf(out, "PAGEMAP\n");
    fprintf(out, " addr : pfn soft-dirty file/shared swapped present\n");

    char path[BUFSIZ];
    snprintf(path, BUFSIZ, "%s/maps", proc);
    FILE *maps = fopen(path, "r");

    snprintf(path, BUFSIZ, "%s/pagemap", proc);
    int pm_fd = open(path, O_RDONLY);

    char buf[BUFSIZ + 1] = "\0";
    int len;

    while ((len = fread(buf, 1, BUFSIZ, maps)) > 0)
    {
        for (int i = 0; i < len; ++i)
            if (buf[i] == 0)
                buf[i] = '\n';
        buf[len] = '\0';

        char *save_row;
        char *row = strtok_r(buf, "\n", &save_row);

        while (row)
        {
            char *addresses = strtok(row, " ");
            char *start_str, *end_str;

            if ((start_str = strtok(addresses, "-")) && (end_str = strtok(NULL, "-")))
            {
                uint64_t start = strtoul(start_str, NULL, 16);
                uint64_t end = strtoul(end_str, NULL, 16);

                for (uint64_t i = start; i < end; i += sysconf(_SC_PAGE_SIZE))
                {
                    uint64_t offset;
                    uint64_t index = i / sysconf(_SC_PAGE_SIZE) + sizeof(offset);

                    pread(pm_fd, &offset, sizeof(offset), index);
                    print_page(i, offset, out);
                }
            }
            row = strtok_r(NULL, "\n", &save_row);
        }
    }
    fclose(maps);
    close(pm_fd);
}
int main(int argc, char **argv)
{
    if (argc != 2)
    {
        puts("Usage: app.exe /proc/[pid]");
        return -1;
    }
    
    check_path(argv[1]);
    const char *link_ptr = (strcmp(argv[1], "self") == 0) ? 
                            LINK_TO_SELF : argv[1];
    FILE *out = fopen("result.txt", "w");
    assert(out);

    cmdline_info(link_ptr, out);
    cwd_info(link_ptr, out);
    environ_info(link_ptr, out);
    stat_info(link_ptr, out);
    statm_info(link_ptr, out);
    fd_info(link_ptr, out);
    exe_info(link_ptr, out);
    root_info(link_ptr, out);
    maps_info(link_ptr, out);
    comm_info(link_ptr, out);
    task_info(link_ptr, out);
    pagemap_info(link_ptr, out);
    fclose(out);
}