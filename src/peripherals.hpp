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
        uint8_t read(uint16_t addr);
        void write(uint16_t addr, uint8_t val);
};


#endif