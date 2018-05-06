#include <xc.h>
#include <stdint.h>
#include <string.h>

// CONFIG1
#pragma config FOSC = INTOSC       // Oscillator Selection (ECH, External Clock, High Power Mode (4-32 MHz): device clock supplied to CLKIN pin)
#pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Memory Code Protection (Data memory code protection is disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable (Brown-out Reset enabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = ON        // Internal/External Switchover (Internal/External Switchover mode is enabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is enabled)

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config VCAPEN = OFF     // Voltage Regulator Capacitor Enable bit (Vcap functionality is disabled on RA6.)
#pragma config PLLEN = ON       // PLL Enable (4x PLL enabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LPBOR = OFF      // Low Power Brown-Out Reset Enable Bit (Low power brown-out is disabled)
#pragma config LVP = ON         // Low-Voltage Programming Enable (Low-voltage programming enabled)


#define BUF_SIZE 64
#define LAYER_SIZE 8

#define set_oe_close() { PORTCbits.RC2 = 1; }
#define set_oe_open() { PORTCbits.RC2 = 0; }
#define set_stcp_low() { PORTCbits.RC1 = 0; }
#define set_stcp_high() { PORTCbits.RC1 = 1; }
#define set_shcp_low() { PORTCbits.RC0 = 0; }
#define set_shcp_high() { PORTCbits.RC0 = 1; }

#define led_up 0
#define led_down 1

uint8_t display_buffer[BUF_SIZE];
uint8_t layer_idx;


void select_layer();
void reset_display();
void delay();
void delay_lack();
void display();

void choose_led(uint8_t x, uint8_t y, uint8_t z, uint8_t state);
void choose_line(uint8_t y, uint8_t z, uint8_t sequence);

void op_L(uint8_t start, uint8_t end, uint8_t state);
void op_O(uint8_t start, uint8_t end, uint8_t state);
void op_V(uint8_t start, uint8_t end, uint8_t state);
void op_E(uint8_t start, uint8_t end, uint8_t state);
void trans_display_love();

void trans_display_heart();
void trans_display_circle();

void op_cell_start();
void op_cell_rotate();
void op_cell_end();

void interrupt timer0() {
    static uint8_t graph_idx;
    if (graph_idx < 10)
        op_cell_start();
    else if (graph_idx < 20)
        op_cell_end();
    else if(graph_idx < 30)
        op_cell_start();
    else if (graph_idx < 93)
        op_cell_rotate();
    else if (graph_idx < 103)
        op_cell_end();
    else if (graph_idx < 113)
        op_cell_start();
    else if (graph_idx < 123)
        op_cell_end();
    else if (graph_idx < 140)
        trans_display_heart();
    else if (graph_idx < 157)
        trans_display_circle();
    else if (graph_idx < 221)
        trans_display_love();
    else if (graph_idx < 238)
        trans_display_circle();
    else if (graph_idx < 255)
        trans_display_heart();
    else;
    
    if(++graph_idx >= 255)
        graph_idx = 0;
    
    TMR0IF = 0;
    
}

void main(void) {
    
    OSCCON = 0b01101011;
    
    TRISA = 0;
    PORTA = 0;
    LATA = 0;
    ANSELA = 0;
    
    TRISC = 0;
    PORTC = 0;
    LATC = 0;

    nWPUEN = 0;
    TMR0CS = 0;
    PSA = 0;
    PS2 = 1;
    PS1 = 1;
    PS0 = 1;
    
    reset_display();
    
    TMR0IF = 0;
    GIE = 1;
    TMR0IE = 1;

    
    for (;;) //扫描循环
    {
        display(); //每次只写一层
    }
}


void select_layer() {
    uint8_t rc;
    rc = LATC & 0b10001111;
    rc |= (layer_idx << 4);
    PORTC = rc;
}

void reset_display() {
    uint8_t i;
    uint8_t start;
    
    layer_idx = 0;
    select_layer();
    
    memset(display_buffer, 0b11111111, BUF_SIZE);
    
    for (i = 0; i < 8; ++i) {
        set_shcp_low();
        PORTA = 0b11111111;
        set_shcp_high();
    }
}

void delay() {
    uint8_t i,j,k;
    for (i = 0; i < 100; ++i)
        for (j = 0; j < 100; ++j);
}

void delay_lack()
{
    uint8_t i,j,k;
    for (i = 0; i < 100; ++i);
}

void display() {
    uint8_t i;
    uint8_t start;
    start = layer_idx * 8;

    set_stcp_low();
    for (i = 0; i < 8; ++i) {
        set_shcp_low();
        PORTA = display_buffer[start++];
        set_shcp_high();
    }
    set_oe_close();
    select_layer();
    set_stcp_high();
    set_oe_open();
    
    ++layer_idx;
    if (layer_idx == 8) {
        layer_idx = 0;
    }
    
    delay_lack();
}


void choose_led(uint8_t x, uint8_t y, uint8_t z, uint8_t state)
{
    if (state == led_up)
    {
        display_buffer[z*8+y] &= ~(1 << x);
    }
    else if (state == led_down)
    {
        display_buffer[z*8+y] |= (1 << x);
    }
    else
    {
        //do nothing
    }
}

void choose_line(uint8_t y, uint8_t z, uint8_t sequence)
{
    
    display_buffer[z*8+y] = sequence;
}


void op_L(uint8_t start, uint8_t end, uint8_t state)
{
    uint8_t y,z;
    if (state == led_up)
    {
        for(y=start;y<=end;y++)
        {
            choose_line(y,0,~0b11100000);
            choose_line(y,1,~0b11100000);
            choose_line(y,2,~0b11100000);
            choose_line(y,3,~0b11100000);
            choose_line(y,4,~0b11100000);
            choose_line(y,5,~0b11111111);
            choose_line(y,6,~0b11111111);
            choose_line(y,7,~0b11111111);
        }
    }
    else if (state == led_down)
    {
        for(y=start;y<=end;y++)
        {
            for(z=0;z<=7;z++)
            {
                choose_line(y,z,~0b00000000);
            }
        }
    }
    else
    {
        //do nothing
    }
}

void op_O(uint8_t start, uint8_t end, uint8_t state)
{
    uint8_t y,z;
    if (state == led_up)
    {
        for(y=start;y<=end;y++)
        {
            choose_line(y,0,~0b00111100);
            choose_line(y,1,~0b01100110);
            choose_line(y,2,~0b11000011);
            choose_line(y,3,~0b11000011);
            choose_line(y,4,~0b11000011);
            choose_line(y,5,~0b11000011);
            choose_line(y,6,~0b01100110);
            choose_line(y,7,~0b00111100);
        }
    }
    else if (state == led_down)
    {
        for(y=start;y<=end;y++)
        {
            for(z=0;z<=7;z++)
            {
                choose_line(y,z,~0b00000000);
            }
        }
    }
    else
    {
        //do nothing
    }
}

void op_V(uint8_t start, uint8_t end, uint8_t state)
{
    uint8_t y,z;
    if (state == led_up)
    {
        for(y=start;y<=end;y++)
        {
            choose_line(y,0,~0b11000011);
            choose_line(y,1,~0b11000011);
            choose_line(y,2,~0b01100110);
            choose_line(y,3,~0b01100110);
            choose_line(y,4,~0b01100110);
            choose_line(y,5,~0b00111100);
            choose_line(y,6,~0b00111100);
            choose_line(y,7,~0b00011000);
        }
    }
    else if (state == led_down)
    {
        for(y=start;y<=end;y++)
        {
            for(z=0;z<=7;z++)
            {
                choose_line(y,z,~0b00000000);
            }
        }
    }
    else
    {
        //do nothing
    }
}

void op_E(uint8_t start, uint8_t end, uint8_t state)
{
    uint8_t y,z;
    if (state == led_up)
    {
        for(y=start;y<=end;y++)
        {
            choose_line(y,0,~0b11111111);
            choose_line(y,1,~0b11111111);
            choose_line(y,2,~0b11100000);
            choose_line(y,3,~0b11111110);
            choose_line(y,4,~0b11111110);
            choose_line(y,5,~0b11100000);
            choose_line(y,6,~0b11111111);
            choose_line(y,7,~0b11111111);
        }
    }
    else if (state == led_down)
    {
        for(y=start;y<=end;y++)
        {
            for(z=0;z<=7;z++)
            {
                choose_line(y,z,~0b00000000);
            }
        }
    }
    else
    {
        //do nothing
    }
}

void trans_display_love()
{
    static uint8_t love_idx;
    if (love_idx<8) //display L
    {
        op_L(0, love_idx, led_up);
    }
    else if (love_idx<16)
    {
        op_L(love_idx-8, love_idx-8, led_down);
    }
    else if (love_idx<24)
    {
        op_O(0, love_idx-16, led_up);
    }
    else if (love_idx<32)
    {
        op_O(love_idx-24, love_idx-24, led_down);
    }
    else if (love_idx<40) //display L
    {
        op_V(0, love_idx-32, led_up);
    }
    else if (love_idx<48)
    {
        op_V(love_idx-40, love_idx-40, led_down);
    }
    else if (love_idx<56)
    {
        op_E(0, love_idx-48, led_up);
    }
    else if (love_idx<64)
    {
        op_V(love_idx-56, love_idx-56, led_down);
    }
    if (++love_idx == 64)
        love_idx = 0;
}


void op_circle(uint8_t start, uint8_t end, uint8_t state)
{
    uint8_t y,z;
    
    if (state == led_up)
    {
        for(y=start;y<=end;y++)
        {
            for(z=start;z<=end;z++)
            {
                if (start==3 && end==4)
                {
                    choose_line(y,z,~0b00011000);
                }
                else if (start==2 && end==5)
                {
                    choose_line(y,z,~0b00111100);
                }
                else if (start==1 && end==6)
                {
                    choose_line(y,z,~0b01111110);
                }
                else if (start==0 && end==7)
                {
                    choose_line(y,z,~0b11111111);
                }
                else
                {
                    //do nothing
                }
            }
        }
    }
    else if (state==led_down)
    {
        for(y=start;y<=end;y++)
        {
            for(z=start;z<=end;z++)
            {
                if (start==3 && end==4)
                {
                    choose_line(y,z,0b00011000);
                }
                else if (start==2 && end==5)
                {
                    choose_line(y,z,0b00111100);
                }
                else if (start==1 && end==6)
                {
                    choose_line(y,z,0b01111110);
                }
                else if (start==0 && end==7)
                {
                    choose_line(y,z,0b11111111);
                }
                else
                {
                    //do nothing
                }
            }
        }
    }
    else;
}

void trans_display_heart() {
    uint8_t y,z;
    static uint8_t graph_idx_heart;
    
    if (graph_idx_heart < 2)
    {
        memset(display_buffer, 0b11111111, BUF_SIZE);
        for(y=3;y<=4;y++)
        {
            for(z=3;z<=4;z++)
            {
                choose_line(y,z,~0b00011000);
            }
        }
    }
    else if (graph_idx_heart < 4)
    {
        for(y=2;y<=5;y++)
        {
            for(z=2;z<=5;z++)
            {
                choose_line(y,z,~0b00111100);
            }
        }
    }
    else if (graph_idx_heart < 6)
    {
        for(y=1;y<=6;y++)
        {
            for(z=1;z<=6;z++)
            {
                choose_line(y,z,~0b01111110);
            }
        }
    }
    else if (graph_idx_heart < 8)
    {
        for(y=0;y<=7;y++)
        {
            for(z=0;z<=7;z++)
            {
                choose_line(y,z,~0b11111111);
            }
        }
    }
    else if (graph_idx_heart < 9);
    //wait
    else if (graph_idx_heart < 11)
    {
        for(y=1;y<=6;y++)
        {
            for(z=1;z<=6;z++)
            {
                choose_line(y,z,~0b01111110);
            }
        }
        for(z=0;z<=7;z++)
        {
            choose_line(0,z,~0b00000000);
            choose_line(7,z,~0b00000000);
        }
        for(y=1;y<=6;y++)
        {
            choose_line(y,0,~0b00000000);
            choose_line(y,7,~0b00000000);
        }
    }
    else if (graph_idx_heart < 13)
    {
        for(y=2;y<=5;y++)
        {
            for(z=2;z<=5;z++)
            {
                choose_line(y,z,~0b00111100);
            }
        }
        for(z=1;z<=6;z++)
        {
            choose_line(1,z,~0b00000000);
            choose_line(6,z,~0b00000000);
        }
        for(y=2;y<=5;y++)
        {
            choose_line(y,1,~0b00000000);
            choose_line(y,6,~0b00000000);
        }
    }
    else if (graph_idx_heart < 15)
    {
        for(y=3;y<=4;y++)
        {
            for(z=3;z<=4;z++)
            {
                choose_line(y,z,~0b00011000);
            }
        }
        for(z=2;z<=5;z++)
        {
            choose_line(2,z,~0b00000000);
            choose_line(5,z,~0b00000000);
        }
        for(y=3;y<=4;y++)
        {
            choose_line(y,2,~0b00000000);
            choose_line(y,5,~0b00000000);
        }
    }
    else if (graph_idx_heart < 17)
    {
        for(y=3;y<=4;y++)
        {
            for(z=3;z<=4;z++)
            {
                choose_line(y,z,~0b00000000);
            }
        }
    }
    
    if (++graph_idx_heart >= 17) {
        graph_idx_heart = 0;
    }
}

void trans_display_circle() {
    uint8_t y,z;
    static uint8_t graph_idx_circle;
    
    if (graph_idx_circle < 2)
    {
        for(y=3;y<=4;y++)
        {
            for(z=3;z<=4;z++)
            {
                choose_line(y,z,~0b00011000);
            }
        }
    }
    else if (graph_idx_circle < 4)
    {
        for(y=3;y<=4;y++)
        {
            for(z=3;z<=4;z++)
            {
                choose_line(y,z,~0b00100100);
            }
        }
        for(y=2;y<=5;y++)
        {
            choose_line(y,2,~0b00111100);
            choose_line(y,5,~0b00111100);
        }
        for(z=3;z<=4;z++)
        {
            choose_line(2,z,~0b00111100);
            choose_line(5,z,~0b00111100);
        }
    }
    else if (graph_idx_circle < 6)
    {
        for(y=2;y<=5;y++)
        {
            for(z=2;z<=5;z++)
            {
                choose_line(y,z,~0b01000010);
            }
        }
        for(y=1;y<=6;y++)
        {
            choose_line(y,1,~0b01111110);
            choose_line(y,6,~0b01111110);
        }
        for(z=2;z<=5;z++)
        {
            choose_line(1,z,~0b01111110);
            choose_line(6,z,~0b01111110);
        }
    }
    else if (graph_idx_circle < 8)
    {
        for(y=1;y<=6;y++)
        {
            for(z=1;z<=6;z++)
            {
                choose_line(y,z,~0b1000001);
            }
        }
        for(y=0;y<=7;y++)
        {
            choose_line(y,0,~0b11111111);
            choose_line(y,7,~0b11111111);
        }
        for(z=1;z<=6;z++)
        {
            choose_line(0,z,~0b11111111);
            choose_line(7,z,~0b11111111);
        }
    }
    else if (graph_idx_circle < 9);
    //wait
    else if (graph_idx_circle < 11)
    {
        for(y=2;y<=5;y++)
        {
            for(z=2;z<=5;z++)
            {
                choose_line(y,z,~0b01000010);
            }
        }
        for(z=1;z<=6;z++)
        {
            choose_line(1,z,~0b01111110);
            choose_line(6,z,~0b01111110);
        }
        for(y=2;y<=5;y++)
        {
            choose_line(y,1,~0b01111110);
            choose_line(y,6,~0b01111110);
        }
        for(z=0;z<=7;z++)//暗
        {
            choose_line(0,z,~0b00000000);
            choose_line(7,z,~0b00000000);
        }
        for(y=1;y<=6;y++)
        {
            choose_line(y,0,~0b00000000);
            choose_line(y,7,~0b00000000);
        }
    }
    else if (graph_idx_circle < 13)
    {
        for(y=3;y<=4;y++)
        {
            for(z=3;z<=4;z++)
            {
                choose_line(y,z,~0b00100100);
            }
        }
        for(z=2;z<=5;z++)
        {
            choose_line(2,z,~0b00111100);
            choose_line(5,z,~0b00111100);
        }
        for(y=3;y<=4;y++)
        {
            choose_line(y,2,~0b00111100);
            choose_line(y,5,~0b00111100);
        }
        for(z=1;z<=6;z++)//暗
        {
            choose_line(1,z,~0b00000000);
            choose_line(6,z,~0b00000000);
        }
        for(y=2;y<=5;y++)
        {
            choose_line(y,1,~0b00000000);
            choose_line(y,6,~0b00000000);
        }
    }
    else if (graph_idx_circle < 15)
    {
        for(z=3;z<=4;z++)
        {
            choose_line(3,z,~0b00011000);
            choose_line(4,z,~0b00011000);
        }
        for(z=2;z<=5;z++)//暗
        {
            choose_line(2,z,~0b00000000);
            choose_line(5,z,~0b00000000);
        }
        for(y=3;y<=4;y++)
        {
            choose_line(y,2,~0b00000000);
            choose_line(y,5,~0b00000000);
        }
    }
    else if (graph_idx_circle < 17)
    {
        for(y=3;y<=4;y++)//暗
        {
            for(z=3;z<=4;z++)
            {
                choose_line(y,z,~0b00000000);
            }
        }
    }
    
    if (++graph_idx_circle >= 17) {
        graph_idx_circle = 0;
    }
}


void op_cell_start()
{
    uint8_t y,z;
    static uint8_t cell_start_idx;
    
    if (cell_start_idx < 2)
    {
        for (y=0;y<8;y++)
        {
            for (z=0;z<8;z++)
            {
                choose_line(y,z,~0b00000000);
            }
        }
    }
    else if (cell_start_idx < 4)
    {
        for (y=0;y<3;y++)
        {
            for (z=0;z<8;z++)
            {
                choose_line(y,z,~0b00000000);
            }
        }
        for (y=3;y<5;y++)
        {
            for (z=0;z<8;z++)
            {
                choose_line(y,z,~0b00011000);
            }
        }
        for (y=5;y<8;y++)
        {
            for (z=0;z<8;z++)
            {
                choose_line(y,z,~0b00000000);
            }
        }
    }
    else if (cell_start_idx < 6)
    {
        for (y=0;y<2;y++)
        {
            for (z=0;z<8;z++)
            {
                choose_line(y,z,~0b00000000);
            }
        }
        for (y=2;y<6;y++)
        {
            for (z=0;z<8;z++)
            {
                choose_line(y,z,~0b00111100);
            }
        }
        for (y=6;y<8;y++)
        {
            for (z=0;z<8;z++)
            {
                choose_line(y,z,~0b00000000);
            }
        }
    }
    else if (cell_start_idx < 8)
    {
        for (y=0;y<1;y++)
        {
            for (z=0;z<8;z++)
            {
                choose_line(y,z,~0b00000000);
            }
        }
        for (y=1;y<7;y++)
        {
            for (z=0;z<8;z++)
            {
                choose_line(y,z,~0b01100110);
            }
        }
        for (y=7;y<8;y++)
        {
            for (z=0;z<8;z++)
            {
                choose_line(y,z,~0b00000000);
            }
        }
    }
    else if (cell_start_idx < 10)
    {
        for (y=0;y<2;y++)
        {
            for (z=0;z<8;z++)
            {
                choose_line(y,z,~0b11000011);
            }
        }
        for (y=2;y<6;y++)
        {
            for (z=0;z<8;z++)
            {
                choose_line(y,z,~0b00000000);
            }
        }
        for (y=6;y<8;y++)
        {
            for (z=0;z<8;z++)
            {
                choose_line(y,z,~0b11000011);
            }
        }
    }
    
    if(++cell_start_idx >= 10)
        cell_start_idx = 0;
}

void op_cell_rotate()
{
    uint8_t y,z;
    static uint8_t cell_rotate_idx;
    
    if (cell_rotate_idx < 1)
    {
        for (y=0;y<2;y++)
        {
            for (z=0;z<8;z++)
            {
                choose_line(y,z,~0b11000011);
            }
        }
        for (y=2;y<6;y++)
        {
            for (z=0;z<8;z++)
            {
                choose_line(y,z,~0b00000000);
            }
        }
        for (y=6;y<8;y++)
        {
            for (z=0;z<8;z++)
            {
                choose_line(y,z,~0b11000011);
            }
        }
    }
    else if (cell_rotate_idx < 2)
    {
        for (z=0;z<8;z++)
        {
            choose_line(0,z,~0b01100000);
            choose_line(1,z,~0b01100011);
            choose_line(2,z,~0b00000011);
            choose_line(3,z,~0b00000000);
            choose_line(4,z,~0b00000000);
            choose_line(5,z,~0b11000000);
            choose_line(6,z,~0b11000110);
            choose_line(7,z,~0b00000110);
        }
    }
    else if (cell_rotate_idx < 3)
    {
        for (z=0;z<8;z++)
        {
            choose_line(0,z,~0b00110000);
            choose_line(1,z,~0b00110000);
            choose_line(2,z,~0b00000011);
            choose_line(3,z,~0b00000011);
            choose_line(4,z,~0b11000000);
            choose_line(5,z,~0b11000000);
            choose_line(6,z,~0b00001100);
            choose_line(7,z,~0b00001100);
        }
    }
    else if (cell_rotate_idx < 4)
    {
        for (z=0;z<8;z++)
        {
            choose_line(0,z,~0b00011000);
            choose_line(1,z,~0b00011000);
            choose_line(2,z,~0b00000000);
            choose_line(3,z,~0b11000011);
            choose_line(4,z,~0b11000011);
            choose_line(5,z,~0b00000000);
            choose_line(6,z,~0b00011000);
            choose_line(7,z,~0b00011000);
        }
    }
    else if (cell_rotate_idx < 5)
    {
        for (z=0;z<8;z++)
        {
            choose_line(0,z,~0b00001100);
            choose_line(1,z,~0b00001100);
            choose_line(2,z,~0b11000000);
            choose_line(3,z,~0b11000000);
            choose_line(4,z,~0b00000011);
            choose_line(5,z,~0b00000011);
            choose_line(6,z,~0b00110000);
            choose_line(7,z,~0b00110000);
        }
    }
    else if (cell_rotate_idx < 6)
    {
        for (z=0;z<8;z++)
        {
            choose_line(0,z,~0b00000110);
            choose_line(1,z,~0b11000110);
            choose_line(2,z,~0b11000000);
            choose_line(3,z,~0b00000000);
            choose_line(4,z,~0b00000000);
            choose_line(5,z,~0b00000011);
            choose_line(6,z,~0b01100011);
            choose_line(7,z,~0b01100000);
        }
    }
//    else if (cell_rotate_idx < 7)
//    {
//        for (z=0;z<8;z++)
//        {
//            choose_line(0,z,~0b00000110);
//            choose_line(1,z,~0b11000110);
//            choose_line(2,z,~0b11000000);
//            choose_line(3,z,~0b00000000);
//            choose_line(4,z,~0b00000000);
//            choose_line(5,z,~0b00000011);
//            choose_line(6,z,~0b01100011);
//            choose_line(7,z,~0b01100000);
//        }
//    }
    if (++cell_rotate_idx >= 7)
        cell_rotate_idx = 0;
}

void op_cell_end()
{
    uint8_t y,z;
    static uint8_t cell_end_idx;
    
    if (cell_end_idx < 2)
    {
        for (y=0;y<2;y++)
        {
            for (z=0;z<8;z++)
            {
                choose_line(y,z,~0b11000011);
            }
        }
        for (y=2;y<6;y++)
        {
            for (z=0;z<8;z++)
            {
                choose_line(y,z,~0b00000000);
            }
        }
        for (y=6;y<8;y++)
        {
            for (z=0;z<8;z++)
            {
                choose_line(y,z,~0b11000011);
            }
        }
    }
    else if (cell_end_idx < 4)
    {
        for (y=0;y<1;y++)
        {
            for (z=0;z<8;z++)
            {
                choose_line(y,z,~0b00000000);
            }
        }
        for (y=1;y<7;y++)
        {
            for (z=0;z<8;z++)
            {
                choose_line(y,z,~0b01100110);
            }
        }
        for (y=7;y<8;y++)
        {
            for (z=0;z<8;z++)
            {
                choose_line(y,z,~0b00000000);
            }
        }
    }
    else if (cell_end_idx < 6)
    {
        for (y=0;y<2;y++)
        {
            for (z=0;z<8;z++)
            {
                choose_line(y,z,~0b00000000);
            }
        }
        for (y=2;y<6;y++)
        {
            for (z=0;z<8;z++)
            {
                choose_line(y,z,~0b00111100);
            }
        }
        for (y=6;y<8;y++)
        {
            for (z=0;z<8;z++)
            {
                choose_line(y,z,~0b00000000);
            }
        }
    }
    else if (cell_end_idx < 8)
    {
        for (y=0;y<3;y++)
        {
            for (z=0;z<8;z++)
            {
                choose_line(y,z,~0b00000000);
            }
        }
        for (y=3;y<5;y++)
        {
            for (z=0;z<8;z++)
            {
                choose_line(y,z,~0b00011000);
            }
        }
        for (y=5;y<8;y++)
        {
            for (z=0;z<8;z++)
            {
                choose_line(y,z,~0b00000000);
            }
        }
    }
    else if (cell_end_idx < 10)
    {
        for (y=0;y<8;y++)
        {
            for (z=0;z<8;z++)
            {
                choose_line(y,z,~0b00000000);
            }
        }
    }
    
    if(++cell_end_idx >= 10)
        cell_end_idx = 0;
}
