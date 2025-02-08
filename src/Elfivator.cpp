#include <iostream>
#include <fstream>
#include <elf.h>
#include <libelf.h>
#include <gelf.h>
#include <fcntl.h>
#include <unistd.h>
#include <iomanip>
#include "Elfivator.h"


/********************/
/* PUBLIC FUNCTIONS */
/********************/


/**
 * Constructor
 * @param filein - Path to a file to load.
 * @return a new Elfivator
 */
Elfivator::Elfivator(const std::string& filein) {
    readElfFile(filein);
}


/**********************/
/* PRIVATE FUNCTIONS  */
/**********************/


/**
 * Parse an ELF file for its important sections.
 * @param filein - Path to ELF file to parse.
 */
void Elfivator::readElfFile(const std::string& filein) {
    // Initialize libelf
    if (elf_version(EV_CURRENT) == EV_NONE) {
        std::cerr << "Libelf initialization failed: " << elf_errmsg(-1) << std::endl;
        return;
    }

    // Open the ELF file
    int fd = open(filein.c_str(), O_RDONLY);
    if (fd == -1) {
        std::cerr << "Failed to open ELF file: " << filein << std::endl;
        return;
    }

    // Initialize ELF descriptor
    Elf *e = elf_begin(fd, ELF_C_READ, nullptr);
    if (!e) {
        std::cerr << "Failed to read ELF header: " << elf_errmsg(-1) << std::endl;
        return;
    }

    // Get ELF header and section headers
    GElf_Ehdr ehdr;
    if (gelf_getehdr(e, &ehdr) == nullptr) {
        std::cerr << "Failed to get ELF header: " << elf_errmsg(-1) << std::endl;
        return;
    }

    // Fetch the entry point address
    entry = ehdr.e_entry;

    // Iterate over sections to find the .text section
    Elf_Scn *scn = nullptr;
    GElf_Shdr shdr;
    while ((scn = elf_nextscn(e, scn)) != nullptr) {
        gelf_getshdr(scn, &shdr);

        // Check if the section has bits
        if(!(shdr.sh_type & SHT_NOBITS)) {
            Elf_Data *data = elf_getdata(scn, nullptr);
            const char *sectionName = elf_strptr(e, ehdr.e_shstrndx, shdr.sh_name);

            // Begrudingly copy the bytes to a vector
            std::vector<unsigned char> dataVec;
            for (size_t i = 0; i < data->d_size; i++)
                dataVec.push_back(((unsigned char *)data->d_buf)[i]);

            // Save important information
            ElfSection section = {
                dataVec,
                std::string(sectionName),
                shdr.sh_addr,
                data->d_size
            };
            sections.push_back(section);
        }
    }

    // Clean up
    elf_end(e);
    close(fd);
}
