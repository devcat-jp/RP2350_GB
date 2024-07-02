#include <Arduino.h>
#include <hardware/structs/systick.h>
#include <RP2040_PIO_GFX.h>
#include "peripherals.hpp"
#include "cpu.hpp"


// ピンアサイン
#define TFT_MOSI 19
#define TFT_CLK 18
#define TFT_DC 22
#define TFT_RST 26
#define TFT_CS 27
// 解像度
#define WIDTH 240
#define HEIGHT 240
// クラス生成
RP2040_PIO_GFX::Gfx gfx;
Peripherals mmio;
Cpu cpu;

// 時刻取得関数
inline uint32_t get_cvr()
{
  return systick_hw->cvr;
}


// debug
uint8_t isBOOTSEL = 0;

char _buf[20];
void dispFunc(){
    // 描画指示
    gfx.swap();
    // 全画面クリア
    gfx.clear(gfx.BLACK);

    // 描画
    mmio.ppu.render_bg(WIDTH, HEIGHT, gfx.getWriteBuffer()); 

    if(isBOOTSEL == 0){
      snprintf(_buf, 8, "%X", cpu.regs.pc);
      gfx.writeFont8(0, 0, "PC:");
      gfx.writeFont8(4, 0, _buf);
      snprintf(_buf, 8, "%X", cpu.ctx.opecode);
      gfx.writeFont8(10, 0, "OP:");
      gfx.writeFont8(14, 0, _buf);
      //
      snprintf(_buf, 8, "%X", cpu.regs.a);
      gfx.writeFont8(0, 1, "A:");
      gfx.writeFont8(3, 1, _buf);
      snprintf(_buf, 8, "%X", cpu.regs.b);
      gfx.writeFont8(0, 2, "B:");
      gfx.writeFont8(3, 2, _buf);
      snprintf(_buf, 8, "%X", cpu.regs.c);
      gfx.writeFont8(0, 3, "C:");
      gfx.writeFont8(3, 3, _buf);
      snprintf(_buf, 8, "%X", cpu.regs.d);
      gfx.writeFont8(0, 4, "D:");
      gfx.writeFont8(3, 4, _buf);
      //
      snprintf(_buf, 8, "%X", cpu.regs.e);
      gfx.writeFont8(10, 1, "E:");
      gfx.writeFont8(13, 1, _buf);
      snprintf(_buf, 8, "%X", cpu.regs.f);
      gfx.writeFont8(10, 2, "F:");
      gfx.writeFont8(13, 2, _buf);
      snprintf(_buf, 8, "%X", cpu.regs.h);
      gfx.writeFont8(10, 3, "H:");
      gfx.writeFont8(13, 3, _buf);
      snprintf(_buf, 8, "%X", cpu.regs.l);
      gfx.writeFont8(10, 4, "L:");
      gfx.writeFont8(13, 4, _buf);
      //
      snprintf(_buf, 16, "%d", cpu.cycle);
      gfx.writeFont8(0, 5, "CY:");
      gfx.writeFont8(4, 5, _buf);
      //snprintf(_buf, 16, "%X", cpu.regs.sp);
      //gfx.writeFont8(10, 5, "SP:");
      //gfx.writeFont8(13, 5, _buf);
      //
      snprintf(_buf, 16, "%d", mmio.ppu.dVal);
      gfx.writeFont8(0, 7, "DE:");
      gfx.writeFont8(4, 7, _buf);
    } else if(isBOOTSEL == 1) {
      // hram表示
      uint8_t _cnt = 0;
      for(int s = 0; s < 16; s++){
        for(int i = 0; i < 8; i++){
          snprintf(_buf, 16, "%X", mmio.hram.read(_cnt));
          gfx.writeFont8(i * 3, s, _buf);
          _cnt++;
        }
      }
    } else if(isBOOTSEL == 2){
      // vram表示
      
      uint16_t color = 0;
      for(int r = 0; r < HEIGHT; r++){
        for(int c = 0; c < WIDTH; c++){
          uint16_t tile_idx = mmio.ppu.get_tile_idx_from_tile_map(0, r >> 3, c >> 3);
          uint8_t pixel = mmio.ppu.get_pixel_from_tile(tile_idx, r & 7, c & 7);
          switch (0xFC >> (pixel << 1) & 0b11)
          {
              case 0b00: color = 0xFFFF; break;
              case 0b01: color = 0xAD55; break;
              case 0b10: color = 0x52AA; break;
              default: color = 0x0000; break;
          }
          uint16_t *p = gfx.getWriteBuffer();
          p[(WIDTH * r) + c] = color;

        }
      }
      
      /*
      uint16_t _cnt = 0;
      for(int s = 0; s < 25; s++){
        for(int i = 0; i < 15; i++){
          snprintf(_buf, 16, "%X", mmio.ppu.read(0x8000 + 1118 + _cnt));
          gfx.writeFont8(i * 2, s, _buf);
          _cnt++;
        }
      }
      */
      
    }
}

void setup() {
  //Serial.begin(9600);

  // LED点灯
  pinMode(25, OUTPUT);
  digitalWrite(25, HIGH);
}


bool my_debug = false;
unsigned long ts = 0, te = 0;
void loop() {
  for(;;){
    ts = get_cvr();
    cpu.emulate_cycle(mmio);

    te = get_cvr();
    mmio.ppu.dVal = ts - te;
    //unsigned long _time = te - ts;
    //Serial.println(_time);

    //delay(500);
  }
}




void setup1(){
   
  // LCD初期化
  gfx.initILI9341(
    TFT_CLK,
    TFT_MOSI,
    TFT_DC,
    TFT_RST,
    TFT_CS,
    40,
    0,
    WIDTH,
    HEIGHT,
    2);

  // ダブルバッファ用のメモリ生成
  // メモリサイズはLCD初期化の際の画面サイズと同じ
  gfx.initDoubleBuffer();

  // DMAによる画像メモリ転送を有効化
  gfx.initDMA();
  //gfx.initDMA(dispFunc);
}

void loop1(){
  /*
  if(BOOTSEL){
    delay(500);
    isBOOTSEL += 1;
    if(isBOOTSEL > 3) isBOOTSEL = 0;
  }
  */
  
  
  if(gfx.isCompletedTransfer()){
    dispFunc();
  }
  
}