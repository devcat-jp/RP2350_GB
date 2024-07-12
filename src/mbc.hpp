#ifndef MBC_HPP
#define MBC_HPP
#include <Arduino.h>

enum class MbcType {
    NoMbc,
    Mbc1,
};


class Mbc{
    private:
        bool bank_mode = false;
        uint16_t low_bank = 0x00;
        uint16_t high_bank = 0x00;
        uint8_t bank_size = 0;
    public:
        bool sram_enable = false;
        MbcType mbc;

        // 初期化
        inline void setup(uint8_t cartridge_type, uint8_t bank_size){
            switch (cartridge_type){
                case 0x00:
                case 0x08:
                case 0x09:
                    // NoMBC
                    this->mbc = MbcType::NoMbc;
                    break;
                case 0x01:
                case 0x02:
                case 0x03:
                    // MBC1
                    this->mbc = MbcType::Mbc1;
                    this->low_bank = 0b00001;       // 下位5bitのみ書き込まれる、1で初期化が必要
                    this->high_bank = 0b00;         // 下位2bitのみ書き込まれる
                    this->bank_size = bank_size;
                    break;
            }
        }

        // MBCへの書き込み処理
        inline void write(uint16_t addr, uint8_t val){
            switch (this->mbc){
                case MbcType::NoMbc:
                case MbcType::Mbc1:
                    // SRAM
                    if(0x0000 <= addr && addr <= 0x1FFF){
                        // 下位4bitに0xAを書き込むとSRAMが有効になる
                         if((val & 0xF) == 0xA) this->sram_enable = true;
                    }
                    // LOWバンクレジスタ
                    else if(0x2000 <= addr && addr <= 0x3FFF){
                        // 下位5bitのみ使用、ただし値が0の場合は1を書く
                        if((val & 0b11111) == 0) this->low_bank = 0b00001;
                        else this->low_bank = val & 0b11111;
                    }
                    // HIGHバンクレジスタ
                    else if(0x4000 <= addr && addr <= 0x5FFF){
                        // 下位2bitのみ使用
                        this->high_bank = val & 0b11;
                    }
                    // バンクモード
                    else if(0x6000 <= addr && addr <= 0x7FFF){
                        if((val & 0b1) > 0) this->bank_mode = true;
                    }
                    break;
            }
        }

        // カートリッジ内のアドレス取得
        inline uint16_t get_addr(uint16_t addr){
            switch (this->mbc){
                case MbcType::NoMbc:
                case MbcType::Mbc1:
                    // 読み出しのみ
                    if(0x0000 <= addr && addr <= 0x3FFF){
                        // バンクモードが有効か？
                        if(this->bank_mode){
                            return ((this->high_bank << 19) | (addr & 0x3FFF));
                        } else {
                            return (addr & 0x3FFF);
                        }
                    }
                    // 読み出しのみ
                    else if(0x4000 <= addr && addr <= 0x7FFF){
                        return (this->high_bank << 19) | ((this->low_bank & (this->bank_size -1)) << 14) | (addr & 0x3FFF);
                    }
                    // 読み書き
                    else if(0xA000 <= addr && addr <= 0xBFFF){
                        // バンクモードが有効か？
                        if(this->bank_mode){
                            return ((this->high_bank << 13) | (addr & 0x1FFF));
                        } else {
                            return (addr & 0x1FFF);
                        }
                    }
                    break;
            }
            return 0xFF;
        }


};





#endif