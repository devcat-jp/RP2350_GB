#include <Arduino.h>
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

// debug
bool isBOOTSEL = false;

char _buf[20];
void dispFunc(){
    Serial.begin(9600);

    // 描画指示
    gfx.swap();
    // 全画面クリア
    gfx.clear(gfx.BLACK);

    if(!isBOOTSEL){
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
      snprintf(_buf, 16, "%X", cpu.regs.sp);
      gfx.writeFont8(10, 5, "SP:");
      gfx.writeFont8(13, 5, _buf);
      //
      snprintf(_buf, 16, "%X", cpu.dVal);
      gfx.writeFont8(0, 7, "DE:");
      gfx.writeFont8(4, 7, _buf);
    } else {
      // hram表示
      uint8_t _cnt = 0;
      for(int s = 0; s < 16; s++){
        for(int i = 0; i < 8; i++){
          snprintf(_buf, 16, "%X", mmio.hram.read(_cnt));
          gfx.writeFont8(i * 3, s, _buf);
          _cnt++;
        }
      }
    }
}

void setup() {
  Serial.begin(9600); 

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
  //gfx.initDMA();
  gfx.initDMA(dispFunc);

  // LED点灯
  pinMode(25, OUTPUT);
  digitalWrite(25, HIGH);
}



bool my_debug = false;
void loop() {


  cpu.emulate_cycle(mmio);

  if(cpu.ctx.opecode == 0xc9) my_debug = true;
  if(my_debug) delay(2000);


  if(BOOTSEL){
    delay(500);
    if(isBOOTSEL) isBOOTSEL = false;
    else isBOOTSEL = true;
  }


}