#ifndef PERIPHERALS_HPP
#define PERIPHERALS_HPP

// 周辺機器管理
#include "bootrom.hpp"
#include "hram.hpp"
#include "wram.hpp"
#include "ppu.hpp"
#include "cartridge.hpp"
#include "interrupts.hpp"

class Peripherals {
    private:
        BootRom bootrom;
        Cartridge *p_cart;
    public:
        WRam wram;
        HRam hram;
        Ppu ppu;

        // 初期化
        inline void setup(Cartridge *p_cart){
            this->p_cart = p_cart;
        }

        // MMIOのリード処理
        inline uint8_t read(Interrupts interrupts, uint16_t addr){
            // bootrom
            if(0x0000 <= addr && addr <= 0x00FF) {
                if(this->bootrom.isActive()){
                    return this->bootrom.read(addr);
                } else {
                    return this->p_cart->read(addr);
                }
            }
            else if (0x0000 <= addr && addr <= 0x7FFF) return this->p_cart->read(addr);     // cart
            else if (0xC000 <= addr && addr <= 0xFDFF) return this->wram.read(addr);        // wram
            else if (0xA000 <= addr && addr <= 0xBFFF) return this->p_cart->read(addr);     // cart
            else if (0x8000 <= addr && addr <= 0x9FFF) return this->ppu.read(addr);         // ppu
            else if (0xFE00 <= addr && addr <= 0xFE9F) return this->ppu.read(addr);         // ppu
            else if (0xFF40 <= addr && addr <= 0xFF4B) return this->ppu.read(addr);         // ppu
            else if (0xFF80 <= addr && addr <= 0xFFFE) return this->hram.read(addr);        // hram
            else if (0xFF0F == addr && addr == 0xFFFF) return interrupts.read(addr);        // interrupts
            else return 0xFF;
        }

        // MMIOのライト処理
        inline void write(Interrupts interrupts, uint16_t addr, uint8_t val){
            // bootrom
            if(0xFF50 == addr) {
                this->bootrom.write(addr, val);
            }
            else if (0x8000 <= addr && addr <= 0x9FFF) this->ppu.write(addr, val);          // ppu
            else if (0xC000 <= addr && addr <= 0xFDFF) this->wram.write(addr, val);         // wram
            else if (0xFE00 <= addr && addr <= 0xFE9F) this->ppu.write(addr, val);          // ppu
            else if (0xFF40 <= addr && addr <= 0xFF4B) this->ppu.write(addr, val);          // ppu
            else if (0xFF80 <= addr && addr <= 0xFFFE) this->hram.write(addr, val);         // hram
            else if (0xFF0F == addr && addr == 0xFFFF) interrupts.write(addr, val);         // interrupts
        }

};


#endif