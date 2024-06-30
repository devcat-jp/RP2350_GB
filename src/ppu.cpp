#include <Arduino.h>
#include "ppu.hpp"

Ppu::Ppu(){
    this->mode = Mode::HBlank;
    this->width = 160;
    this->height = 144;
}

uint8_t Ppu::read(uint16_t addr){
    if(0x8000 <= addr && addr <= 0x9FFF) {
        // モード3の時はVRAMにアクセスできない
        //if(this->mode == Mode::Drawing) return 0xFF;
        //else return this->vram[addr & 0x1FFF];
        return this->vram[addr & 0x1FFF];
    } else if(0xFE00 <= addr && addr <= 0xFE9F){
        // モード2・3の時はOAMにアクセスできない
        //if(this->mode == Mode::Drawing || this->mode == Mode::OamScan) return 0xFF;
        //else return this->oam[addr & 0xFF];
        return this->oam[addr & 0xFF];
    } 
    else if(0xFF40 == addr) return this->lcdc;
    else if(0xFF41 == addr) return 0x80 | this->stat | this->mode;
    else if(0xFF42 == addr) return this->scy;
    else if(0xFF43 == addr) return this->scx;
    else if(0xFF44 == addr) return this->ly;
    else if(0xFF45 == addr) return this->lyc;
    else if(0xFF47 == addr) return this->bgp;
    else if(0xFF48 == addr) return this->obp0;
    else if(0xFF49 == addr) return this->obp1;
    else if(0xFF4A == addr) return this->wy;
    else if(0xFF4B == addr) return this->wx;

    return 0xFF;
}

void Ppu::write(uint16_t addr, uint8_t val){
    if(0x8000 <= addr && addr <= 0x9FFF) {
        // モード3の時はVRAMにアクセスできない
        //if(this->mode != Mode::Drawing){
        //    this->vram[addr & 0x1FFF] = val;
        //}
        this->vram[addr & 0x1FFF] = val;
    } else if(0xFE00 <= addr && addr <= 0xFE9F){
        // モード2・3の時はOAMにアクセスできない
        //if(this->mode != Mode::Drawing && this->mode != Mode::OamScan) {
        //    this->oam[addr & 0xFF] = val;
        //}
        this->oam[addr & 0xFF] = val;
    } 
    else if(0xFF40 == addr) this->lcdc = val;
    else if(0xFF41 == addr) this->stat = (this->stat & LYC_EQ_LY) | (val & 0xF0);
    else if(0xFF42 == addr) this->scy = val;
    else if(0xFF43 == addr) this->scx = val;
    else if(0xFF44 == addr) {}
    else if(0xFF45 == addr) this->lyc = val;
    else if(0xFF47 == addr) this->bgp = val;
    else if(0xFF48 == addr) this->obp0 = val;
    else if(0xFF49 == addr) this->obp1 = val;
    else if(0xFF4A == addr) this->wy = val;
    else if(0xFF4B == addr) this->wx = val;
}


// 特定タイルの特定ピクセルデータを取得する
uint8_t Ppu::get_pixel_from_tile(uint16_t tile_idx, uint8_t row, uint8_t col){
    uint16_t r = (uint16_t)(row * 2);                               // タイルは1行（8pix）あたり16bit
    uint16_t c = (uint16_t)(7 - col);                               // col列目は（7-col）bit目
    uint16_t tile_addr = tile_idx << 4;                             // タイルの開始アドレスはタイルのインデックスの16倍
    uint8_t low = this->vram[(tile_addr | r)];                      // ピクセルの上位bit（8ピクセル分）
    uint8_t high = this->vram[(tile_addr | (r+1))];                 // ピクセルの上位bit（8ピクセル分）
    return (((high >> c) & 1) << 1) | ((low >> c) & 1);
}


// タイルマップの特定マスに格納されたタイルのインデックスを取得する
// 0x1800～0x1BFF と 0x1C00～0x1FF の2つのタイルマップがある
uint16_t Ppu::get_tile_idx_from_tile_map(bool tile_map, uint8_t row, uint8_t col){
    uint16_t start_addr = 0x1800 | (tile_map << 10);                // tile_mapが有効な場合は　0x1C00
    uint16_t ret = this->vram[(start_addr | ((row << 5) + col))];
    if((this->lcdc & TILE_DATA_ADDRESSING_MODE) > 0) {               // lcdcの4bit目が0の場合はそのまま
        return ret;
    } else {
        return (uint16_t)((int16_t)ret + 0x100);                    // 符号付変数に変換し、0x100を加算した後に符号なしに戻す
    }
}

// bgのレンダリング
void Ppu::render_bg(uint16_t lcd_width, uint16_t lcd_height, uint16_t *pBuffer){
    if(this->lcdc & BG_WINDOW_ENABLE == 0) return;

    uint16_t color = 0;

    for(uint8_t r = 0; r < this->height; r++){
        uint8_t y = r + this->scy;

        for(uint8_t c = 0; c < this->width; c++){
            uint8_t x = c + this->scx;

            // タイルインデックスを取得
            uint16_t tile_idx = this->get_tile_idx_from_tile_map((this->lcdc & BG_TILE_MAP) > 0, y >> 3, x >> 3);

            // 色取得
            uint8_t pixel = this->get_pixel_from_tile(tile_idx, y & 7, x & 7);
            switch (this->bgp >> (pixel << 1) & 0b11)
            {
                case 0b00: color = 0xFFFF; break;
                case 0b01: color = 0xAD55; break;
                case 0b10: color = 0x52AA; break;
                default: color = 0x0000; break;
            }

            pBuffer[(lcd_width * r) + c] = color;
        }
    }
}