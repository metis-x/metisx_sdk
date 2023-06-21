#pragma once

#include <elf.h>

namespace metisx
{
namespace api
{
namespace util
{
class ElfLoader
{
    public:
        ElfLoader(uint32_t muType, const char* fileName);
        ~ElfLoader();
    public:
        bool textOnly;

    private:
        uint8_t* _buf;
        uint64_t _size;
        uint64_t _startAddr;
        uint64_t _endAddr;

    private:
        Elf64_Ehdr  _elfHeader64;        /* elf-header is fixed size */
        Elf64_Shdr* _sectionHeader64Ptr; /* section-header table is variable size */

    public:
        uint8_t* getBuf(void);
        uint64_t getSize(void);

    private:
        void     readElfHeader(int32_t fd);
        void     readSectionHeaderTable(int32_t fd);
        uint32_t getSectionIndex(int32_t fd, const char* sectionName);
        char*    readSection(int32_t fd, Elf64_Shdr sectionHeader);
        void     getSectionFromMuElf(uint32_t muType, const char* fileName);

        // for debugging
        void printElfHeader(void);
        void printSectionHeaders(int32_t fd);
};
} // namespace util
} // namespace api
} // namespace metisx
