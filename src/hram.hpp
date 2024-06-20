#ifndef HRAM_HPP
#define HRAM_HPP

// 128byteのRAM

class HRam{
    private:
        uint8_t hram[128];
    public:
        uint8_t read(uint16_t addr);
        void write(uint16_t addr, uint8_t val);
};

#endif