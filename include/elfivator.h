#ifndef ELFIVATOR_H
#define ELFIVATOR_H

#include <cstddef>
#include <vector>

class Elfivator {
private:
    class ElfSection {
    public:
        std::vector<unsigned char>data;
        std::string name;
        size_t offset;
        size_t size;
    };

    /**
     * Parse the contents of an elf into this
     * @param filein - Path to file to parse
     */
    void readElfFile(const std::string& filein);

public:
    /* New class members */
    std::vector<ElfSection> sections;
    size_t entry;

    /**
     * Constructor
     * @param filein - Path to a file to load.
     * @return a new Elfivator
     */
    Elfivator(const std::string& filein);
};

#endif
