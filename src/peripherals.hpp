#ifndef PERIPHERALS_HPP
#define PERIPHERALS_HPP

// 周辺機器管理
#include "bootrom.hpp"
#include "hram.hpp"

class Peripherals {
    private:
        BootRom bootrom;
        HRam hram;
    public:
        uint8_t read(uint16_t addr);
        void write(uint16_t addr, uint8_t val);
};


#endif