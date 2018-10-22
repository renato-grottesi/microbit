#include "mbed.h"
#include <PwmOut.h>

class SuspendedPlotter
{
public:
    SuspendedPlotter();
    void startDraw();
    void stopDraw();
    void moveTo(float x, float y);
    void unrollLeft(int steps);
    void unrollRight(int steps);
    void rollLeft(int steps);
    void rollRight(int steps);

private:
    class Stepper
    {
    public:
        Stepper(PinName pn1, PinName pn2, PinName pn3, PinName pn4);
        void stepTo(int dir);

    private:
        int m_step;
        DigitalOut m_in1;
        DigitalOut m_in2;
        DigitalOut m_in3;
        DigitalOut m_in4;
    };

    class Servo
    {
    public:
        Servo(PinName pn1);
        void open();
        void close();
        void reset();

    private:
        PwmOut m_pwm;
        int m_period;
        int m_pulse_width;
    };

private:
    Stepper m_left;
    Stepper m_right;
    Servo m_servo;
    float m_x;
    float m_y;
};
