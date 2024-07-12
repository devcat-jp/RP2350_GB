#ifndef CARTRIDGE_HPP
#define CARTRIDGE_HPP
#include <Arduino.h>
#include "mbc.hpp"

// ソフトの構造体
struct CartridgeHeader {
    uint8_t entry_point[4];
    uint8_t logo[48];
    uint8_t title[11];
    uint8_t maker[4];
    uint8_t cgb_flag;
    uint8_t new_license[2];
    uint8_t sgb_flag;
    uint8_t cartridge_type;
    uint8_t rom_size;
    uint8_t sram_size;
    uint8_t destination;
    uint8_t old_license;
    uint8_t game_version;
    uint8_t header_checksum;
    uint8_t global_checksum[2];
};


class Cartridge {
    private:
        uint32_t rom_size = 0;
        uint32_t sram_size = 0;
        uint8_t bank_size = 0;
        uint8_t *sram;
        uint8_t *p_rom;
        Mbc mbc;

    public:
        //
        CartridgeHeader header;
        //
        inline void loadRom(uint8_t *pRom){
            this->p_rom = pRom;

            memcpy(&this->header, &pRom[0x100], 0x50);                      // ヘッダ情報のコピー
            this->rom_size = 1 << (15 + this->header.rom_size);             // ROM容量
            switch (this->header.sram_size) {                               // ROM内臓SRAM
                case 0x00: this->sram_size = 0; break;
                case 0x01: this->sram_size = 0x800; break;
                case 0x02: this->sram_size = 0x2000; break;
                case 0x03: this->sram_size = 0x8000; break;
                case 0x04: this->sram_size = 0x20000; break;
                case 0x05: this->sram_size = 0x10000; break;
            }
            this->sram = new uint8_t[this->sram_size];
            this->bank_size = this->rom_size >> 14;                         // ROMバンクは1つあたり16KB
            this->mbc.setup(this->header.cartridge_type, this->bank_size);
        }

        // Read
        inline uint8_t read(uint16_t addr) {
            if(0x0000 <= addr && addr <= 0x7FFF) {
                return this->p_rom[this->mbc.get_addr(addr)];
            } 
            else if(0xA000 <= addr && addr <= 0xBFFF) {
                switch(this->mbc.mbc){
                    case MbcType::NoMbc:
                        return this->sram[addr];
                    case MbcType::Mbc1:
                        if(this->mbc.sram_enable) return this->sram[this->mbc.get_addr(addr)];
                        else return 0xFF;
                }
            }
            return 0xFF;
        }

        // Write
        inline void write(uint16_t addr, uint8_t val) {
            if(0x0000 <= addr && addr <= 0x7FFF) {
                this->mbc.write(addr, val);
            } 
            else if(0xA000 <= addr && addr <= 0xBFFF) {
                switch(this->mbc.mbc){
                    case MbcType::NoMbc:
                        this->sram[addr] = val;
                    case MbcType::Mbc1:
                        if(this->mbc.sram_enable) this->sram[this->mbc.get_addr(addr)] = val;
                }
            }
        }
};





#endif