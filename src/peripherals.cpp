
#include <Arduino.h>
#include "peripherals.hpp"

uint8_t Peripherals::read(uint16_t addr){
    // bootrom
    if(0x0000 <= addr && addr <= 0x00FF) {
        if(this->bootrom.isActive()){
            return this->bootrom.read(addr);
        } else {
            return 0xFF;
        }
    // hram
    } else if (0xFF80 <= addr && addr <= 0xFFFE){
        return this->hram.read(addr);
    // それ以外
    } else {
        return 0xFF;
    }
}


void Peripherals::write(uint16_t addr, uint8_t val){
    // bootrom
    if(0xFF50 == addr) {
        this->bootrom.write(addr, val);
    } else if (0xFF80 <= addr && addr <= 0xFFFE){
        this->hram.write(addr, val);
    }

}
