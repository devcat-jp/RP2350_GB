#ifndef WRAM_HPP
#define WRAM_HPP


class WRam{
    private:
        uint8_t wram[0x8000];
    public:
        inline uint8_t read(uint16_t addr){
            return this->wram[addr & 0x1fff];
        }

        inline void write(uint16_t addr, uint8_t val){
            this->wram[addr & 0x1fff] = val;
        }

};

#endif