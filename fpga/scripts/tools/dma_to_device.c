
/*
 * This file is part of the Xilinx DMA IP Core driver tools for Linux
 *
 * Copyright (c) 2016-present,  Xilinx, Inc.
 * All rights reserved.
 *
 * This source code is licensed under BSD-style license (found in the
 * LICENSE file in the root directory of this source tree)
 */

#include <fcntl.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include "../../xdma_driver/cdev_sgdma.h"

#include "dma_utils.c"

static struct option const long_opts[] = {
    {"device",       required_argument, NULL, 'd'},
    {"user_config_bar",       required_argument, NULL, 'u'},
    {"address",      required_argument, NULL, 'a'},
    {"aperture",     required_argument, NULL, 'k'},
    {"size",         required_argument, NULL, 's'},
    {"offset",       required_argument, NULL, 'o'},
    {"count",        required_argument, NULL, 'c'},
    {"data infile",  required_argument, NULL, 'f'},
    {"data outfile", required_argument, NULL, 'w'},
    {"help",         no_argument,       NULL, 'h'},
    {"verbose",      no_argument,       NULL, 'v'},
    {"init ddr",     no_argument,       NULL, 'i'},
    {0,              0,                 0,    0  }
};

#define DEVICE_NAME_DEFAULT         "/dev/xdma0_h2c_0"
#define DEVICE_USER_BAR_DEFAULT    "/dev/xdma0_user"
#define SIZE_DEFAULT        (32)
#define COUNT_DEFAULT       (1)

#define KB(x)                          (x * 1024)
#define MB(x)                          (KB(x) * 1024)
#define GB(x)                          (MB(x) * 1024)
#define METIS_DDR_BASE                 (0x0800000000ull)
#define MTS_CORE_DDR_OFFSET            (0x1000000000ull)
#define XDMA_USER_ADDR_MAP_SIZE        (MB(64))
#define FPGA_CONFIG_ADDR_OFFSET        (0x80010ull)

typedef union
{
    struct
    {
        uint32_t is_ddr_64 : 1;
        uint32_t : 3;
        uint32_t num_metis_core : 4;
        uint32_t : 8;
        uint32_t sim : 1;
        uint32_t : 15;
    }st;
    uint32_t u32;
}FpgaConfig_t;


static int test_dma(char* devname, char* user_bar, uint64_t addr, uint64_t aperture, uint64_t size, uint64_t offset, uint64_t count, char* infname, char* ofname, uint8_t init_ddr, uint64_t start1GBOffset);

static void usage(const char* name)
{
    int i = 0;

    fprintf(stdout, "%s\n\n", name);
    fprintf(stdout, "usage: %s [OPTIONS]\n\n", name);
    fprintf(stdout,
            "Write via SGDMA, optionally read input from a file.\n\n");

    fprintf(stdout, "  -%c (--%s) device (defaults to %s)\n", long_opts[i].val, long_opts[i].name, DEVICE_NAME_DEFAULT);
    i++;
    fprintf(stdout, "  -%c (--%s) user_bar (defaults to %s)\n", long_opts[i].val, long_opts[i].name, DEVICE_USER_BAR_DEFAULT);
    i++;
    fprintf(stdout, "  -%c (--%s) the start address on the AXI bus\n", long_opts[i].val, long_opts[i].name);
    i++;
    fprintf(stdout, "  -%c (--%s) memory address aperture\n", long_opts[i].val, long_opts[i].name);
    i++;
    fprintf(stdout,
            "  -%c (--%s) size of a single transfer in bytes, default %d,\n",
            long_opts[i].val,
            long_opts[i].name,
            SIZE_DEFAULT);
    i++;
    fprintf(stdout, "  -%c (--%s) page offset of transfer\n", long_opts[i].val, long_opts[i].name);
    i++;
    fprintf(stdout, "  -%c (--%s) number of transfers, default %d\n", long_opts[i].val, long_opts[i].name, COUNT_DEFAULT);
    i++;
    fprintf(stdout, "  -%c (--%s) filename to read the data from.\n", long_opts[i].val, long_opts[i].name);
    i++;
    fprintf(stdout,
            "  -%c (--%s) filename to write the data of the transfers\n",
            long_opts[i].val,
            long_opts[i].name);
    i++;
    fprintf(stdout, "  -%c (--%s) print usage help and exit\n", long_opts[i].val, long_opts[i].name);
    i++;
    fprintf(stdout, "  -%c (--%s) verbose output\n", long_opts[i].val, long_opts[i].name);
    i++;
    fprintf(stdout, "  -%c (--%s) init dram\n", long_opts[i].val, long_opts[i].name);
    i++;

    fprintf(stdout, "\nReturn code:\n");
    fprintf(stdout, "  0: all bytes were dma'ed successfully\n");
    fprintf(stdout, "  < 0: error\n\n");
}

int main(int argc, char* argv[])
{
    int      cmd_opt;
    char*    device   = DEVICE_NAME_DEFAULT;
    char*    user_bar = DEVICE_USER_BAR_DEFAULT;
    uint64_t address  = 0;
    uint64_t aperture = 0;
    uint64_t size     = SIZE_DEFAULT;
    uint64_t offset   = 0;
    uint64_t count    = COUNT_DEFAULT;
    char*    infname  = NULL;
    char*    ofname   = NULL;
    uint8_t  init_ddr = 0;
    uint64_t start1GBOffset = 0;

    while ((cmd_opt = getopt_long(argc, argv, "vhc:f:d:u:a:k:s:o:w:i:x:", long_opts, NULL)) != -1)
    {
        switch (cmd_opt)
        {
        case 0:
            /* long option */
            break;
        case 'd':
            /* device node name */
            // fprintf(stdout, "'%s'\n", optarg);
            device = strdup(optarg);
            break;
        case 'u':
            user_bar = strdup(optarg);
            break;
        case 'a':
            /* RAM address on the AXI bus in bytes */
            address = getopt_integer(optarg);
            break;
        case 'k':
            /* memory aperture windows size */
            aperture = getopt_integer(optarg);
            break;
        case 's':
            /* size in bytes */
            size = getopt_integer(optarg);
            break;
        case 'o':
            offset = getopt_integer(optarg) & 4095;
            break;
            /* count */
        case 'c':
            count = getopt_integer(optarg);
            break;
            /* count */
        case 'f':
            infname = strdup(optarg);
            break;
        case 'w':
            ofname = strdup(optarg);
            break;
            /* print usage help and exit */
        case 'v':
            verbose = 1;
            break;
        case 'i':
            init_ddr = 1;
            break;
        case 'x':   // 1GB Offset
            start1GBOffset = getopt_integer(optarg);
            break;
        case 'h':
        default:
            usage(argv[0]);
            exit(0);
            break;
        }
    }

    if (verbose)
        fprintf(stdout,
                "dev %s, addr 0x%lx, aperture 0x%lx, size 0x%lx, offset 0x%lx, "
                "count %lu\n",
                device,
                address,
                aperture,
                size,
                offset,
                count);

    return test_dma(device, user_bar, address, aperture, size, offset, count, infname, ofname, init_ddr, start1GBOffset);
}

static int test_dma(char* devname, char* user_bar, uint64_t addr, uint64_t aperture, uint64_t size, uint64_t offset, uint64_t count, char* infname, char* ofname, uint8_t init_ddr, uint64_t start1GBOffset)
{
    uint64_t        i;
    ssize_t         rc;
    size_t          bytes_done = 0;
    size_t          out_offset = 0;
    char*           buffer     = NULL;
    char*           allocated  = NULL;
    struct timespec ts_start, ts_end;
    int             infile_fd  = -1;
    int             outfile_fd = -1;
    int             fpga_fd    = open(devname, O_RDWR);
    long            total_time = 0;
    float           result;
    float           avg_time  = 0;
    int             underflow = 0;
    

    if (fpga_fd < 0)
    {
        fprintf(stderr, "unable to open device %s, %d.\n", devname, fpga_fd);
        perror("open device");
        return -EINVAL;
    }

    if (infname)
    {
        infile_fd = open(infname, O_RDONLY);
        if (infile_fd < 0)
        {
            fprintf(stderr, "unable to open input file %s, %d.\n", infname, infile_fd);
            perror("open input file");
            rc = -EINVAL;
            goto out;
        }
    }

    if (ofname)
    {
        outfile_fd = open(ofname, O_RDWR | O_CREAT | O_TRUNC | O_SYNC, 0666);
        if (outfile_fd < 0)
        {
            fprintf(stderr, "unable to open output file %s, %d.\n", ofname, outfile_fd);
            perror("open output file");
            rc = -EINVAL;
            goto out;
        }
    }

    if (init_ddr)
    {
        int             user_fd    = open(user_bar, O_RDWR);
        void*           user_bar_base_addr = mmap(NULL, XDMA_USER_ADDR_MAP_SIZE - 4, PROT_READ | PROT_WRITE, MAP_SHARED, user_fd, 0);

        if (user_bar_base_addr == (void*)-1)
        {
            fprintf(stderr, "DDR INIT, Failed to mmap config bar!\n");
            goto out;
        }

        uint64_t fpga_config_addr = (uint64_t)user_bar_base_addr + FPGA_CONFIG_ADDR_OFFSET;
        FpgaConfig_t fpgaConfig = {0,};

        fpgaConfig.u32 = *(volatile uint32_t*)(fpga_config_addr);

        size              = GB(1);
        offset            = MB(1);
        uint64_t startOff = MB(1);
        uint64_t endOff   = MB(1);
        uint32_t numMetisCore = 0;

        if (!fpgaConfig.st.is_ddr_64) {
            count = 32;
        }
        else {
            count = 64;
        }

        if (!fpgaConfig.st.num_metis_core)
        {
            numMetisCore = 1;
        }
        else
        {
            numMetisCore = 2;
        }

        posix_memalign((void**)&allocated, 4096 /*alignment */, size + 4096);
        if (!allocated)
        {
            fprintf(stderr, "OOM %lu.\n", size + 4096);
            rc = -ENOMEM;
            goto out;
        }
        buffer = allocated;

        memset(buffer, 0, size);

        rc = clock_gettime(CLOCK_MONOTONIC, &ts_start);

        for (uint32_t metisCoreId = 0; metisCoreId < numMetisCore; metisCoreId++)
        {
            addr = METIS_DDR_BASE + MTS_CORE_DDR_OFFSET * metisCoreId;
            addr += startOff;
            for (int i = start1GBOffset; i < count; i++)
            {
                size = GB(1);
                if (i == 0)
                {
                    size -= startOff;
                }
                else if (i == count - 1)
                {
                    size -= endOff;
                }
                rc = write_from_buffer(devname, fpga_fd, buffer, size, addr);
                if (rc < 0)
                    goto out;

                fprintf(stderr,"MTS Core#%d, Write #%dGB @ %lx..\n", metisCoreId, i + 1, addr);
                addr += size;
            }
            fprintf(stderr,"\n");
        }
        rc = clock_gettime(CLOCK_MONOTONIC, &ts_end);

        timespec_sub(&ts_end, &ts_start);
        total_time += ts_end.tv_nsec;
        printf(
            "#%lu: CLOCK_MONOTONIC %ld.%09ld sec. write\n",
            i,
            ts_end.tv_sec,
            ts_end.tv_nsec);
        
        close(user_fd);
    }
    else
    {
        posix_memalign((void**)&allocated, 4096 /*alignment */, size + 4096);
        if (!allocated)
        {
            fprintf(stderr, "OOM %lu.\n", size + 4096);
            rc = -ENOMEM;
            goto out;
        }
        buffer = allocated + offset;
        if (verbose)
            fprintf(stdout, "host buffer 0x%lx = %p\n", size + 4096, buffer);

        if (infile_fd >= 0)
        {
            rc = read_to_buffer(infname, infile_fd, buffer, size, 0);
            if (rc < 0 || rc < size)
                goto out;
        }

        for (i = 0; i < count; i++)
        {
            /* write buffer to AXI MM address using SGDMA */
            rc = clock_gettime(CLOCK_MONOTONIC, &ts_start);

            if (aperture)
            {
                struct xdma_aperture_ioctl io;

                io.buffer   = (unsigned long)buffer;
                io.len      = size;
                io.ep_addr  = addr;
                io.aperture = aperture;
                io.done     = 0UL;

                rc = ioctl(fpga_fd, IOCTL_XDMA_APERTURE_W, &io);
                if (rc < 0 || io.error)
                {
                    fprintf(stdout,
                            "#%d: aperture W ioctl failed %d,%d.\n",
                            i,
                            rc,
                            io.error);
                    goto out;
                }

                bytes_done = io.done;
            }
            else
            {
                rc = write_from_buffer(devname, fpga_fd, buffer, size, addr);
                if (rc < 0)
                    goto out;

                bytes_done = rc;
            }

            rc = clock_gettime(CLOCK_MONOTONIC, &ts_end);

            if (bytes_done < size)
            {
                printf("#%d: underflow %ld/%ld.\n",
                       i,
                       bytes_done,
                       size);
                underflow = 1;
            }

            /* subtract the start time from the end time */
            timespec_sub(&ts_end, &ts_start);
            total_time += ts_end.tv_nsec;
            /* a bit less accurate but side-effects are accounted for */
            if (verbose)
                fprintf(stdout,
                        "#%lu: CLOCK_MONOTONIC %ld.%09ld sec. write %ld bytes\n",
                        i,
                        ts_end.tv_sec,
                        ts_end.tv_nsec,
                        size);

            if (outfile_fd >= 0)
            {
                rc = write_from_buffer(ofname, outfile_fd, buffer, bytes_done, out_offset);
                if (rc < 0 || rc < bytes_done)
                    goto out;
                out_offset += bytes_done;
            }
        }

        if (!underflow)
        {
            avg_time = (float)total_time / (float)count;
            result   = ((float)size) * 1000 / avg_time;
            if (verbose)
                printf("** Avg time device %s, total time %ld nsec, avg_time = %f, size = %lu, BW = %f \n",
                       devname,
                       total_time,
                       avg_time,
                       size,
                       result);
            printf("%s ** Average BW = %lu, %f\n", devname, size, result);
        }
    }

out:
    close(fpga_fd);
    if (infile_fd >= 0)
        close(infile_fd);
    if (outfile_fd >= 0)
        close(outfile_fd);
    free(allocated);

    if (rc < 0)
        return rc;
    /* treat underflow as error */
    return underflow ? -EIO : 0;
}
