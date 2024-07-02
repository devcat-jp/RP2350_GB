#ifndef HRAM_HPP
#define HRAM_HPP

// 128byteのRAM

class HRam{
    private:
        uint8_t hram[128];
    public:
        inline uint8_t read(uint16_t addr){
            return this->hram[addr & 0x7f];
        }

        inline void write(uint16_t addr, uint8_t val){
            this->hram[addr & 0x7f] = val;
        }

};

#endif