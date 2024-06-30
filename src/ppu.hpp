#ifndef PPU_HPP
#define PPU_HPP

// LCDCレジスタで使用する定数
const uint8_t PPU_ENABLE = 1 << 7;
const uint8_t WINDOW_TILE_MAP = 1 << 6;
const uint8_t WINDOW_ENABLE = 1 << 5;
const uint8_t TILE_DATA_ADDRESSING_MODE = 1 << 4;
const uint8_t BG_TILE_MAP = 1 << 3;
const uint8_t SPRITE_SIZE = 1 << 2;
const uint8_t SPRITE_ENABLE = 1 << 1;
const uint8_t BG_WINDOW_ENABLE = 1 << 0;

// STATレジスタで使用する定数
const uint8_t LYC_EQ_LY_INT = 1 << 6;
const uint8_t QAM_SCAN_INT = 1 << 5;
const uint8_t VBLANK_INT = 1 << 4;
const uint8_t HBLANK_INT = 1 << 3;
const uint8_t LYC_EQ_LY = 1 << 2;

enum Mode {
    HBlank = 0,
    VBlank = 1,
    OamScan = 2,
    Drawing = 3,
};

class Ppu {
    private:
        uint8_t width;
        uint8_t height;
        Mode mode;
        uint8_t lcdc;
        uint8_t stat;
        uint8_t scx;        // スクロール用のレジスタ
        uint8_t scy;
        uint8_t ly;         // 現在描画している行目、書き込み不可
        uint8_t lyc;        // ly と lyc が一致した際に割り込みが発生
        uint8_t bgp;        // bg / window用
        uint8_t obp0;       // sprite用
        uint8_t obp1;       // 
        uint8_t wx;         // windowの左上座標
        uint8_t wy;         //
        uint8_t vram[0x2000];
        uint8_t oam[0xa0];
    public:
        uint8_t dVal;
        Ppu();
        uint8_t read(uint16_t addr);
        void write(uint16_t addr, uint8_t val);
        uint8_t get_pixel_from_tile(uint16_t tile_idx, uint8_t row, uint8_t col);
        uint16_t get_tile_idx_from_tile_map(bool tile_map, uint8_t row, uint8_t col);
        void render_bg(uint16_t lcd_width, uint16_t lcd_height,  uint16_t *pBuffer);


};

#endif