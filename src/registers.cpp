#include <Arduino.h>
#include "registers.hpp"


Registers::Registers(){
    this->a = 0;
    this->b = 0;
    this->c = 0;
    this->d = 0;
    this->e = 0;
    this->f = 0;
    this->h = 0;
    this->l = 0;
    this->pc = 0;
    this->sp = 0;
}


// AFの組み合わせのレジスタ、Fが下位ビット
uint16_t Registers::af(){
    return uint16_t(this->a << 8 | this->f);
}

// BCの組み合わせのレジスタ、Cが下位ビット
uint16_t Registers::bc(){
    return uint16_t(this->b << 8 | this->c);
}

 // DEの組み合わせのレジスタ、Eが下位ビット
uint16_t Registers::de(){
    return uint16_t(this->d << 8 | this->e);
}

// HLの組み合わせのレジスタ、Lが下位ビット
uint16_t Registers::hl(){
    return uint16_t(this->h << 8 | this->l);
}

// AFへの書き込み
void Registers::write_af(uint16_t val){
    this->a = val >> 8;
    this->f = val & 0xF0;   // Fの下位4bitは未使用で常に0らしい
}

// BCへの書き込み
void Registers::write_bc(uint16_t val){
    this->b = val >> 8;
    this->c = val;
}

// DEへの書き込み
void Registers::write_de(uint16_t val){
    this->d = val >> 8;
    this->e = val;
}

// HLへの書き込み
void Registers::write_hl(uint16_t val){
    this->h = val >> 8;
    this->l = val;
}

// F（フラグレジスタ）を取得する
// Z（7bit目） 演算結果が0の場合に1になる
bool Registers::zf(){
    return (this->f & 0b1000'0000) > 0;
}
// N（6bit目） 減算命令の場合に1になる
bool Registers::nf(){
    return (this->f & 0b0100'0000) > 0;
}
// H（5bit目） 3bit目で繰り上がり、繰り下がりが発生すると1になる
bool Registers::hf(){
    return (this->f & 0b0010'0000) > 0;
}
// C（4bit目） 7bit目で繰り上がり（下がり）が発生すると1になる
bool Registers::cf(){
    return (this->f & 0b0001'0000) > 0;
}

// F（フラグレジスタ）をセットする
// Z（7bit目） 演算結果が0の場合に1になる
void Registers::set_zf(bool flag){
    if(flag){
        this->f |= 0b1000'0000;     // 7bit目を立てる
    } else {
        this->f &= 0b0111'1111;     // 7bit目を下げる
    }
}
// N（6bit目） 減算命令の場合に1になる
void Registers::set_nf(bool flag){
    if(flag){
        this->f |= 0b0100'0000;     // 6bit目を立てる
    } else {
        this->f &= 0b1011'1111;     // 6bit目を下げる
    }
}
// H（5bit目） 3bit目で繰り上がり、繰り下がりが発生すると1になる
void Registers::set_hf(bool flag){
    if(flag){
        this->f |= 0b0010'0000;     // 5bit目を立てる
    } else {
        this->f &= 0b1101'1111;     // 5bit目を下げる
    }
}
// C（4bit目） 7bit目で繰り上がり（下がり）が発生すると1になる
void Registers::set_cf(bool flag){
    if(flag){
        this->f |= 0b0001'0000;     // 4bit目を立てる
    } else {
        this->f &= 0b1110'1111;     // 4bit目を下げる
    }
}