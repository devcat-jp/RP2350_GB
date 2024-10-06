#ifndef REGISTERS_HPP
#define REGISTERS_HPP

class Registers{
    private:
    public:
        Registers(){
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
        uint8_t a;
        uint8_t b;
        uint8_t c;
        uint8_t d;
        uint8_t e;
        uint8_t f;
        uint8_t h;
        uint8_t l;
        uint16_t pc;
        uint16_t sp;

        // AFの組み合わせのレジスタ、Fが下位ビット
        uint16_t af(){
            return uint16_t(this->a << 8 | (uint16_t)this->f);
        }

        // BCの組み合わせのレジスタ、Cが下位ビット
        uint16_t bc(){
            return uint16_t(this->b << 8 | (uint16_t)this->c);
        }

        // DEの組み合わせのレジスタ、Eが下位ビット
        uint16_t de(){
            return uint16_t(this->d << 8 | (uint16_t)this->e);
        }

        // HLの組み合わせのレジスタ、Lが下位ビット
        uint16_t hl(){
            return uint16_t(this->h << 8 | (uint16_t)this->l);
        }

        // AFへの書き込み
        inline void write_af(uint16_t val){
            this->a = (uint8_t)(val >> 8);
            this->f = val & 0xF0;   // Fの下位4bitは未使用で常に0らしい
        }

        // BCへの書き込み
        inline void write_bc(uint16_t val){
            this->b = (uint8_t)(val >> 8);
            this->c = (uint8_t)val;
        }

        // DEへの書き込み
        inline void write_de(uint16_t val){
            this->d = (uint8_t)(val >> 8);
            this->e = (uint8_t)val;
        }

        // HLへの書き込み
        inline void write_hl(uint16_t val){
            this->h = (uint8_t)(val >> 8);
            this->l = (uint8_t)val;
        }
        
        // F（フラグレジスタ）を取得する
        // Z（7bit目） 演算結果が0の場合に1になる
        inline bool zf(){
            return (this->f & 0b1000'0000) > 0;
        }
        // N（6bit目） 減算命令の場合に1になる
        inline bool nf(){
            return (this->f & 0b0100'0000) > 0;
        }
        // H（5bit目） 3bit目で繰り上がり、繰り下がりが発生すると1になる
        inline bool hf(){
            return (this->f & 0b0010'0000) > 0;
        }
        // C（4bit目） 7bit目で繰り上がり（下がり）が発生すると1になる
        inline bool cf(){
            return (this->f & 0b0001'0000) > 0;
        }
        // F（フラグレジスタ）をセットする
        // Z（7bit目） 演算結果が0の場合に1になる
        inline void set_zf(bool flag){
            if(flag){
                this->f |= 0b1000'0000;     // 7bit目を立てる
            } else {
                this->f &= 0b0111'1111;     // 7bit目を下げる
            }
        }
        // N（6bit目） 減算命令の場合に1になる
        inline void set_nf(bool flag){
            if(flag){
                this->f |= 0b0100'0000;     // 6bit目を立てる
            } else {
                this->f &= 0b1011'1111;     // 6bit目を下げる
            }
        }
        // H（5bit目） 3bit目で繰り上がり、繰り下がりが発生すると1になる
        inline void set_hf(bool flag){
            if(flag){
                this->f |= 0b0010'0000;     // 5bit目を立てる
            } else {
                this->f &= 0b1101'1111;     // 5bit目を下げる
            }
        }
        // C（4bit目） 7bit目で繰り上がり（下がり）が発生すると1になる
        inline void set_cf(bool flag){
            if(flag){
                this->f |= 0b0001'0000;     // 4bit目を立てる
            } else {
                this->f &= 0b1110'1111;     // 4bit目を下げる
            }
        }
};





#endif