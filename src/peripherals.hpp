#ifndef PERIPHERALS_HPP
#define PERIPHERALS_HPP

// 周辺機器管理
#include "bootrom.hpp"
#include "hram.hpp"
#include "Ppu.hpp"

class Peripherals {
    private:
        BootRom bootrom;
    public:
        HRam hram;
        Ppu ppu;

        // MMIOのリード処理
        inline uint8_t read(uint16_t addr){
            // bootrom
            if(0x0000 <= addr && addr <= 0x00FF) {
                if(this->bootrom.isActive()){
                    return this->bootrom.read(addr);
                } else {
                    return 0xFF;
                }
            }
            else if (0x8000 <= addr && addr <= 0x9FFF) return this->ppu.read(addr);         // ppu
            else if (0xFE00 <= addr && addr <= 0xFE9F) return this->ppu.read(addr);         // ppu
            else if (0xFF40 <= addr && addr <= 0xFF4B) return this->ppu.read(addr);         // ppu
            else if (0xFF80 <= addr && addr <= 0xFFFE) return this->hram.read(addr);        // hram
            else return 0xFF;
        }

        // MMIOのライト処理
        inline void write(uint16_t addr, uint8_t val){
            // bootrom
            if(0xFF50 == addr) {
                this->bootrom.write(addr, val);
            }
            else if (0x8000 <= addr && addr <= 0x9FFF) this->ppu.write(addr, val);          // ppu
            else if (0xFE00 <= addr && addr <= 0xFE9F) this->ppu.write(addr, val);          // ppu
            else if (0xFF40 <= addr && addr <= 0xFF4B) this->ppu.write(addr, val);          // ppu
            else if (0xFF80 <= addr && addr <= 0xFFFE) this->hram.write(addr, val);         // hram
        }

};


#endif