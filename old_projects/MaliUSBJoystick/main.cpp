#include "mbed.h"
#include "USBMouse.h"
#include "USBJoystick.h"

USBJoystick joystick(0x0079, 0x0001);

DigitalIn l_r(p17);
DigitalIn l_l(p18);
DigitalIn l_u(p20);
DigitalIn l_d(p19);
DigitalIn l_b1(p16);
DigitalIn l_b2(p10);
DigitalIn l_b3(p6);
DigitalIn l_b4(p7);
DigitalIn l_b5(p9);
DigitalIn l_b6(p5);

DigitalIn r_r(p22);
DigitalIn r_l(p21);
DigitalIn r_u(p23);
DigitalIn r_d(p24);
DigitalIn r_b1(p11);
DigitalIn r_b2(p29);
DigitalIn r_b3(p13);
DigitalIn r_b4(p12);
DigitalIn r_b5(p15);
DigitalIn r_b6(p14);

DigitalIn c_l(p25);
DigitalIn c_r(p26);
DigitalIn c_u(p27);
DigitalIn c_d(p28);

int main() {
    int16_t x_l = 0;
    int16_t y_l = 0;
    int8_t buttons_l = 0;
    int16_t x_r = 0;
    int16_t y_r = 0;
    int8_t buttons_r = 0;

    while (1) {
        x_l = 0;
        y_l = 0;
        if(l_l) x_l = -127;
        if(l_r) x_l = 127;
        if(l_d) y_l = 127;
        if(l_u) y_l = -127;
        buttons_l = l_b1 | l_b2 << 1 | l_b3 << 2 | l_b4 << 3 | l_b5 << 4 | l_b6 << 5 | r_l << 6 | r_r << 7;

        x_r = 0;
        y_r = 0;
        if(c_l) x_r = -127;
        if(c_r) x_r = 127;
        if(c_d) y_r = 127;
        if(c_u) y_r = -127;
        buttons_r = r_b1 | r_b2 << 1 | r_b3 << 2 | r_b4 << 3 | r_b5 << 4 | r_b6 << 5 | r_u << 6 | r_d << 7;

        joystick.update(x_l, y_l, buttons_l, x_r, y_r, buttons_r);

        wait(0.001);
    }
}