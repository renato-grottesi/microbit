#include "SuspendedPlotter.h"

SuspendedPlotter::Servo::Servo(PinName pn)
    : m_pwm(pn)
    , m_period(20000)
    , m_pulse_width(1700)
{
    m_pwm.period_us(m_period);
    m_pwm.pulsewidth_us(m_pulse_width);
}

void SuspendedPlotter::Servo::open()
{
    // 2500 is max
    while(m_pulse_width < 1700)
    {
        m_pwm.period_us(m_period);
        m_pwm.pulsewidth_us(m_pulse_width);
        wait_ms(50);
        m_pulse_width += 50;
    }
}

void SuspendedPlotter::Servo::close()
{
    while(m_pulse_width > 1200)
    {
        m_pwm.period_us(m_period);
        m_pwm.pulsewidth_us(m_pulse_width);
        wait_ms(50);
        m_pulse_width -= 50;
    }
}

SuspendedPlotter::Stepper::Stepper(PinName pn1, PinName pn2, PinName pn3, PinName pn4)
    : m_step(0)
    , m_in1(pn1)
    , m_in2(pn2)
    , m_in3(pn3)
    , m_in4(pn4)
{
    for (int i = 0; i < 8; i++)
        stepTo(-1);
    for (int i = 0; i < 8; i++)
        stepTo(1);
}

void SuspendedPlotter::Stepper::stepTo(int dir)
{
    static const int steps[8][4] = {
        {1, 0, 0, 0},
        {1, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 1},
        {0, 0, 0, 1},
        {1, 0, 0, 1}
    };
    m_step = (m_step+dir+8) % 8;

    m_in1 = steps[m_step][0];
    m_in2 = steps[m_step][1];
    m_in3 = steps[m_step][2];
    m_in4 = steps[m_step][3];
}

SuspendedPlotter::SuspendedPlotter()
    : m_left(p17, p18, p19, p20)
    , m_right(p13, p14, p15, p16)
    , m_servo(p21)
    , m_x(0.5)
    , m_y(0.5)
{
}

void SuspendedPlotter::startDraw()
{
    m_servo.close();
}

void SuspendedPlotter::stopDraw()
{
    m_servo.open();
}

void SuspendedPlotter::moveTo(float x, float y)
{
    static const float MIN_Y = 0.2;
    static const float MAX_Y = 2.0;
    if (x < 0.0)
        x = 0.0;
    if (x > 1.0)
        x = 1.0;
    if (y < MIN_Y)
        y = MIN_Y;
    if (y > MAX_Y)
        y = MAX_Y;

    /* Adaptation of Bresenham's line algorithm */
    /* 280 mm per 10000 steps */
    /* Base is 815 mm or 29107 steps */
    /* Diagonals at 0.5 are 576mm */ /*280 left , 280 right */
    static const float BASE_STEPS_L = 29107;
    static const float BASE_STEPS_R = 29107;
    int left_0 = hypot(m_x * BASE_STEPS_L, m_y * BASE_STEPS_L);
    int right_0 = hypot(BASE_STEPS_R - m_x * BASE_STEPS_R, m_y * BASE_STEPS_R);
    int left_1 = hypot(x * BASE_STEPS_L, y * BASE_STEPS_L);
    int right_1 = hypot(BASE_STEPS_R - x * BASE_STEPS_R, y * BASE_STEPS_R);

    int d_right = abs(left_1 - left_0);
    int s_right = left_0 < left_1 ? 1 : -1;
    int d_left = abs(right_1 - right_0);
    int s_left = right_0 < right_1 ? 1 : -1;
    int err = (d_right > d_left ? d_right : -d_left) / 2, e2;

    while ((left_0 != left_1) || (right_0 != right_1))
    {
        if (left_0 == left_1 && right_0 == right_1)
            break;
        e2 = err;
        if (e2 > -d_right)
        {
            err -= d_left;
            left_0 += s_right;
            m_left.stepTo(-s_right); /* Should be +, but the left motor is reversed */
            //printf("LEFT: current=%d, left_0=%d, left_1=%d, s_right=%d\n\r", m_left.getPosition(), left_0, left_1, s_right);
        }
        if (e2 < d_left)
        {
            err += d_right;
            right_0 += s_left;
            m_right.stepTo(-s_left);
            //printf("RIGHT: current=%d, right_0=%d, right1=%d, s_left=%d\n\r", m_right.getPosition(), right_0, right_1, s_left);
        }
        wait_ms(3);
    }

    m_x = x;
    m_y = y;
}

void SuspendedPlotter::unrollLeft(int steps)
{
    for (int i = 0; i < steps; i++)
    {
        m_left.stepTo(-1); /* Should be +, but the left motor is reversed */
        wait_ms(2);
    }
}

void SuspendedPlotter::unrollRight(int steps)
{
    for (int i = 0; i < steps; i++)
    {
        m_right.stepTo(-1);
        wait_ms(2);
    }
}

void SuspendedPlotter::rollLeft(int steps)
{
    for (int i = 0; i < steps; i++)
    {
        m_left.stepTo(1); /* Should be -, but the left motor is reversed */
        wait_ms(2);
    }
}

void SuspendedPlotter::rollRight(int steps)
{
    for (int i = 0; i < steps; i++)
    {
        m_right.stepTo(1);
        wait_ms(2);
    }
}
