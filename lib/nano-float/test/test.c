//  Unit Test for nano-float. To run QEMU Emulator for Blue Pill:
//  cd /mnt/c/qemu_stm32 ; arm-softmmu/qemu-system-arm -M stm32-f103c8 -semihosting -kernel /mnt/c/codal-libopencm3/.pioenvs/bluepill_f103c8/firmware.bin
//  cd C:\qemu_stm32 && arm-none-eabi-gdb -ex="target remote localhost:1234" /codal-libopencm3/.pioenvs/bluepill_f103c8/firmware.elf
#ifdef UNIT_TEST
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/rtc.h>
#include <unity.h>
#include <logger.h>
#include <math.h>
#define M_LN10		2.30258509299404568402  //  Natural log of 10
#define M_PI_2		1.57079632679489661923  //  Pi divided by 2
#define pi (M_PI_2 * 2)

extern double __aeabi_ddiv(double n, double d);
extern double __aeabi_dmul(double x, double y);
extern int __aeabi_dcmpeq(double x, double y);
extern int __aeabi_dcmplt(double x, double y);
extern int __aeabi_dcmple(double x, double y);
extern int __aeabi_dcmpge(double x, double y);
extern int __aeabi_dcmpgt(double x, double y);
extern int __aeabi_dcmpun(double x, double y);
extern float  __aeabi_fdiv(float  n, float d);
extern int __aeabi_d2iz(double x);
extern unsigned __aeabi_d2uiz(double x);

extern uint8_t *get_float_usage(uint16_t *size);
volatile double x = 0, y = 0, r = 0;    //  Must use variables x and y because compiler will optimise all constants.
volatile float xf = 0, yf = 0, rf = 0;  //  For float unit tests.

//  All comparisons are accurate up to 0.000001, as defined in platformio.ini:
//  -D UNITY_FLOAT_PRECISION=0.000001 -D UNITY_DOUBLE_PRECISION=0.000001

void test_aeabi_fdiv(void) {
    //  gcc compiles "x / y" to "__aeabi_fdiv(x, y)"
    xf = 2205.1969; yf = 270.8886; rf = xf / yf; TEST_ASSERT_EQUAL_FLOAT( 8.140604292687105, rf );
}

void test_aeabi_ddiv(void) {
    x = 2205.1969; y = 270.8886; r = x / y; TEST_ASSERT_EQUAL_DOUBLE( 8.140604292687105, r );
}

void test_aeabi_dmul(void) {
    x = 2205.1969; y = 270.8886; r = x * y; TEST_ASSERT_EQUAL_DOUBLE( 597362.70096534, r );
}

void test_aeabi(void) {
x = 2205.1969;  y = 270.8886;   r = __aeabi_ddiv(x, y);   TEST_ASSERT_EQUAL_DOUBLE( 8.140604292687105  , r );
x = -2205.1969; y = 270.8886;   r = __aeabi_ddiv(x, y);   TEST_ASSERT_EQUAL_DOUBLE( -8.140604292687105 , r );
x = 2205.1969;  y = -270.8886;  r = __aeabi_ddiv(x, y);   TEST_ASSERT_EQUAL_DOUBLE( -8.140604292687105 , r );
x = -2205.1969; y = -270.8886;  r = __aeabi_ddiv(x, y);   TEST_ASSERT_EQUAL_DOUBLE( 8.140604292687105  , r );
x = 2205.1969;  y = 270.8886;   r = __aeabi_dmul(x, y);   TEST_ASSERT_EQUAL_DOUBLE( 597362.70096534    , r );
x = -2205.1969; y = 270.8886;   r = __aeabi_dmul(x, y);   TEST_ASSERT_EQUAL_DOUBLE( -597362.70096534   , r );
x = 2205.1969;  y = -270.8886;  r = __aeabi_dmul(x, y);   TEST_ASSERT_EQUAL_DOUBLE( -597362.70096534   , r );
x = -2205.1969; y = -270.8886;  r = __aeabi_dmul(x, y);   TEST_ASSERT_EQUAL_DOUBLE( 597362.70096534    , r );
x = 2205.1969;  y = 2205.1969;  r = __aeabi_dcmpeq(x, y); TEST_ASSERT_EQUAL_DOUBLE( 1                  , r );
x = 2205.1969;  y = 2205.1968;  r = __aeabi_dcmpeq(x, y); TEST_ASSERT_EQUAL_DOUBLE( 0                  , r );
x = 2205.1969;  y = 2205.1970;  r = __aeabi_dcmpeq(x, y); TEST_ASSERT_EQUAL_DOUBLE( 0                  , r );
x = 2205.1969;  y = 0;          r = __aeabi_dcmpeq(x, y); TEST_ASSERT_EQUAL_DOUBLE( 0                  , r );
x = -2205.1969; y = -2205.1969; r = __aeabi_dcmpeq(x, y); TEST_ASSERT_EQUAL_DOUBLE( 1                  , r );
x = -2205.1969; y = -2205.1968; r = __aeabi_dcmpeq(x, y); TEST_ASSERT_EQUAL_DOUBLE( 0                  , r );
x = -2205.1969; y = -2205.1970; r = __aeabi_dcmpeq(x, y); TEST_ASSERT_EQUAL_DOUBLE( 0                  , r );
x = -2205.1969; y = 0;          r = __aeabi_dcmpeq(x, y); TEST_ASSERT_EQUAL_DOUBLE( 0                  , r );
x = 2205.1969;  y = 2205.1969;  r = __aeabi_dcmplt(x, y); TEST_ASSERT_EQUAL_DOUBLE( 0                  , r );
x = 2205.1969;  y = 2205.1968;  r = __aeabi_dcmplt(x, y); TEST_ASSERT_EQUAL_DOUBLE( 0                  , r );
x = 2205.1969;  y = 2205.1970;  r = __aeabi_dcmplt(x, y); TEST_ASSERT_EQUAL_DOUBLE( 1                  , r );
x = 2205.1969;  y = 0;          r = __aeabi_dcmplt(x, y); TEST_ASSERT_EQUAL_DOUBLE( 0                  , r );
x = -2205.1969; y = -2205.1969; r = __aeabi_dcmplt(x, y); TEST_ASSERT_EQUAL_DOUBLE( 0                  , r );
x = -2205.1969; y = -2205.1968; r = __aeabi_dcmplt(x, y); TEST_ASSERT_EQUAL_DOUBLE( 1                  , r );
x = -2205.1969; y = -2205.1970; r = __aeabi_dcmplt(x, y); TEST_ASSERT_EQUAL_DOUBLE( 0                  , r );
x = -2205.1969; y = 0;          r = __aeabi_dcmplt(x, y); TEST_ASSERT_EQUAL_DOUBLE( 0                  , r );
x = 2205.1969;  y = 2205.1969;  r = __aeabi_dcmple(x, y); TEST_ASSERT_EQUAL_DOUBLE( 1                  , r );
x = 2205.1969;  y = 2205.1968;  r = __aeabi_dcmple(x, y); TEST_ASSERT_EQUAL_DOUBLE( 0                  , r );
x = 2205.1969;  y = 2205.1970;  r = __aeabi_dcmple(x, y); TEST_ASSERT_EQUAL_DOUBLE( 1                  , r );
x = 2205.1969;  y = 0;          r = __aeabi_dcmple(x, y); TEST_ASSERT_EQUAL_DOUBLE( 0                  , r );
x = -2205.1969; y = -2205.1969; r = __aeabi_dcmple(x, y); TEST_ASSERT_EQUAL_DOUBLE( 1                  , r );
x = -2205.1969; y = -2205.1968; r = __aeabi_dcmple(x, y); TEST_ASSERT_EQUAL_DOUBLE( 1                  , r );
x = -2205.1969; y = -2205.1970; r = __aeabi_dcmple(x, y); TEST_ASSERT_EQUAL_DOUBLE( 0                  , r );
x = -2205.1969; y = 0;          r = __aeabi_dcmple(x, y); TEST_ASSERT_EQUAL_DOUBLE( 0                  , r );
x = 2205.1969;  y = 2205.1969;  r = __aeabi_dcmpge(x, y); TEST_ASSERT_EQUAL_DOUBLE( 1                  , r );
x = 2205.1969;  y = 2205.1968;  r = __aeabi_dcmpge(x, y); TEST_ASSERT_EQUAL_DOUBLE( 1                  , r );
x = 2205.1969;  y = 2205.1970;  r = __aeabi_dcmpge(x, y); TEST_ASSERT_EQUAL_DOUBLE( 0                  , r );
x = 2205.1969;  y = 0;          r = __aeabi_dcmpge(x, y); TEST_ASSERT_EQUAL_DOUBLE( 1                  , r );
x = -2205.1969; y = -2205.1969; r = __aeabi_dcmpge(x, y); TEST_ASSERT_EQUAL_DOUBLE( 1                  , r );
x = -2205.1969; y = -2205.1968; r = __aeabi_dcmpge(x, y); TEST_ASSERT_EQUAL_DOUBLE( 0                  , r );
x = -2205.1969; y = -2205.1970; r = __aeabi_dcmpge(x, y); TEST_ASSERT_EQUAL_DOUBLE( 1                  , r );
x = -2205.1969; y = 0;          r = __aeabi_dcmpge(x, y); TEST_ASSERT_EQUAL_DOUBLE( 0                  , r );
x = 2205.1969;  y = 2205.1969;  r = __aeabi_dcmpgt(x, y); TEST_ASSERT_EQUAL_DOUBLE( 0                  , r );
x = 2205.1969;  y = 2205.1968;  r = __aeabi_dcmpgt(x, y); TEST_ASSERT_EQUAL_DOUBLE( 1                  , r );
x = 2205.1969;  y = 2205.1970;  r = __aeabi_dcmpgt(x, y); TEST_ASSERT_EQUAL_DOUBLE( 0                  , r );
x = 2205.1969;  y = 0;          r = __aeabi_dcmpgt(x, y); TEST_ASSERT_EQUAL_DOUBLE( 1                  , r );
x = -2205.1969; y = -2205.1969; r = __aeabi_dcmpgt(x, y); TEST_ASSERT_EQUAL_DOUBLE( 0                  , r );
x = -2205.1969; y = -2205.1968; r = __aeabi_dcmpgt(x, y); TEST_ASSERT_EQUAL_DOUBLE( 0                  , r );
x = -2205.1969; y = -2205.1970; r = __aeabi_dcmpgt(x, y); TEST_ASSERT_EQUAL_DOUBLE( 1                  , r );
x = -2205.1969; y = 0;          r = __aeabi_dcmpgt(x, y); TEST_ASSERT_EQUAL_DOUBLE( 0                  , r );
x = 2205.1969;  y = 2205.1969;  r = __aeabi_dcmpun(x, y); TEST_ASSERT_EQUAL_DOUBLE( 0                  , r );
x = 2205.1969;  y = 2205.1968;  r = __aeabi_dcmpun(x, y); TEST_ASSERT_EQUAL_DOUBLE( 1                  , r );
x = 2205.1969;  y = 2205.1970;  r = __aeabi_dcmpun(x, y); TEST_ASSERT_EQUAL_DOUBLE( 1                  , r );
x = 2205.1969;  y = 0;          r = __aeabi_dcmpun(x, y); TEST_ASSERT_EQUAL_DOUBLE( 1                  , r );
x = -2205.1969; y = -2205.1969; r = __aeabi_dcmpun(x, y); TEST_ASSERT_EQUAL_DOUBLE( 0                  , r );
x = -2205.1969; y = -2205.1968; r = __aeabi_dcmpun(x, y); TEST_ASSERT_EQUAL_DOUBLE( 1                  , r );
x = -2205.1969; y = -2205.1970; r = __aeabi_dcmpun(x, y); TEST_ASSERT_EQUAL_DOUBLE( 1                  , r );
x = -2205.1969; y = 0;          r = __aeabi_dcmpun(x, y); TEST_ASSERT_EQUAL_DOUBLE( 1                  , r );
xf = 2205.1969;  yf = 270.8886;   rf = __aeabi_fdiv(x, y);   TEST_ASSERT_EQUAL_FLOAT( 8.140604292687105  , r );
xf = -2205.1969; yf = 270.8886;   rf = __aeabi_fdiv(x, y);   TEST_ASSERT_EQUAL_FLOAT( -8.140604292687105 , r );
xf = 2205.1969;  yf = -270.8886;  rf = __aeabi_fdiv(x, y);   TEST_ASSERT_EQUAL_FLOAT( -8.140604292687105 , r );
xf = -2205.1969; yf = -270.8886;  rf = __aeabi_fdiv(x, y);   TEST_ASSERT_EQUAL_FLOAT( 8.140604292687105  , r );
x = 2205.1969;  r = __aeabi_d2iz(x);   TEST_ASSERT_EQUAL_DOUBLE( 2205               , r );
x = -2205.1969; r = __aeabi_d2iz(x);   TEST_ASSERT_EQUAL_DOUBLE( -2205              , r );
x = 2205.1969;  r = __aeabi_d2uiz(x);  TEST_ASSERT_EQUAL_DOUBLE( 2205               , r );
x = -2205.1969; r = __aeabi_d2uiz(x);  TEST_ASSERT_EQUAL_DOUBLE( 2205               , r );
}

void test_sqrt(void) {
//  Autogenerated from https://docs.google.com/spreadsheets/d/1Uogm7SpgWVA4AiP6gqFkluaozFtlaEGMc4K2Mbfee7U/edit#gid=1740497564
x = 100;       r = sqrt(x);     TEST_ASSERT_EQUAL_DOUBLE( 10.000000   , r );
x = 2;         r = sqrt(x);     TEST_ASSERT_EQUAL_DOUBLE( 1.414214    , r );
x = -0;        r = sqrt(x);     TEST_ASSERT_EQUAL_DOUBLE( -0.000000   , r );
// x = -1.0;   r = sqrt(x);     TEST_ASSERT_EQUAL_DOUBLE( -nan        , r );
}

void test_log(void) {
x = 1;         r = log(x);      TEST_ASSERT_EQUAL_DOUBLE( 0.000000    , r );
x = 2;         r = log(x);      TEST_ASSERT_EQUAL_DOUBLE( _M_LN2      , r );
x = 10;        r = log(x);      TEST_ASSERT_EQUAL_DOUBLE( M_LN10      , r );
// x = +Inf;   r = log(x);      TEST_ASSERT_EQUAL_DOUBLE( inf         , r );
// x = 0;      r = log(x);      TEST_ASSERT_EQUAL_DOUBLE( -inf        , r );
}

void test_exp(void) {
x = 1;         r = exp(x);      TEST_ASSERT_EQUAL_DOUBLE( 2.718282    , r );
x = _M_LN2;    r = exp(x);      TEST_ASSERT_EQUAL_DOUBLE( 2           , r );
x = M_LN10;    r = exp(x);      TEST_ASSERT_EQUAL_DOUBLE( 10          , r );
x = -0;        r = exp(x);      TEST_ASSERT_EQUAL_DOUBLE( 1.000000    , r );
// x = -Inf;   r = exp(x);      TEST_ASSERT_EQUAL_DOUBLE( 0.000000    , r );
}

void test_log10(void) {
x = 1000;      r = log10(x);    TEST_ASSERT_EQUAL_DOUBLE( 3.000000    , r );
x = 0.001;     r = log10(x);    TEST_ASSERT_EQUAL_DOUBLE( -3.000000   , r );
x = 1;         r = log10(x);    TEST_ASSERT_EQUAL_DOUBLE( 0.000000    , r );
// x = +Inf;   r = log10(x);    TEST_ASSERT_EQUAL_DOUBLE( inf         , r );
// x = 0;      r = log10(x);    TEST_ASSERT_EQUAL_DOUBLE( -inf        , r );
}

void test_pow(void) {
x = 2;         y = 10;   r = pow(x, y);      TEST_ASSERT_EQUAL_DOUBLE( 1024.000000 , r );
x = 2;         y = 0.5;  r = pow(x, y);      TEST_ASSERT_EQUAL_DOUBLE( 1.414214    , r );
x = -2;        y = -3;   r = pow(x, y);      TEST_ASSERT_EQUAL_DOUBLE( -0.125000   , r );
// x = -1;        y = NAN;  r = pow(x, y);      TEST_ASSERT_EQUAL_DOUBLE( nan         , r );
// x = +1;        y = NAN;  r = pow(x, y);      TEST_ASSERT_EQUAL_DOUBLE( 1.000000    , r );
// x = INFINITY;  y = 2;    r = pow(x, y);      TEST_ASSERT_EQUAL_DOUBLE( inf         , r );
// x = INFINITY;  y = -1;   r = pow(x, y);      TEST_ASSERT_EQUAL_DOUBLE( 0.000000    , r );
// x = -1;        y = 1/3;  r = pow(x, y);      TEST_ASSERT_EQUAL_DOUBLE( -nan        , r );
}

void test_ldexp(void) {
x = 7;         y = -4;   r = ldexp(x, y);    TEST_ASSERT_EQUAL_DOUBLE( 0.437500    , r );
x = -0;        y = 10;   r = ldexp(x, y);    TEST_ASSERT_EQUAL_DOUBLE( -0.000000   , r );
// x = -Inf;   y = -1;   r = ldexp(x, y);    TEST_ASSERT_EQUAL_DOUBLE( -inf        , r );
// x = 1;      y = 1024; r = ldexp(x, y);    TEST_ASSERT_EQUAL_DOUBLE( inf         , r );
}

void test_sin(void) {
x = pi/6;      r = sin(x);      TEST_ASSERT_EQUAL_DOUBLE( 0.500000    , r );
x = pi/2;      r = sin(x);      TEST_ASSERT_EQUAL_DOUBLE( 1.000000    , r );
x = -3*pi/4;   r = sin(x);      TEST_ASSERT_EQUAL_DOUBLE( -0.707107   , r );
x = +0;        r = sin(x);      TEST_ASSERT_EQUAL_DOUBLE( 0.000000    , r );
x = -0;        r = sin(x);      TEST_ASSERT_EQUAL_DOUBLE( -0.000000   , r );
// x = INFINITY;  r = sin(x);   TEST_ASSERT_EQUAL_DOUBLE( -nan        , r );
}

void test_cos(void) {
x = pi/3;      r = cos(x);      TEST_ASSERT_EQUAL_DOUBLE( 0.500000    , r );
x = pi/2;      r = cos(x);      TEST_ASSERT_EQUAL_DOUBLE( 0.000000    , r );
x = -3*pi/4;   r = cos(x);      TEST_ASSERT_EQUAL_DOUBLE( -0.707107   , r );
x = +0;        r = cos(x);      TEST_ASSERT_EQUAL_DOUBLE( 1.000000    , r );
x = -0;        r = cos(x);      TEST_ASSERT_EQUAL_DOUBLE( 1.000000    , r );
// x = INFINITY;  r = cos(x);   TEST_ASSERT_EQUAL_DOUBLE( -nan        , r );
}

void test_tan(void) {
x = 3*pi/4;    r = tan(x);      TEST_ASSERT_EQUAL_DOUBLE( -1.000000   , r );
x = 5*pi/4;    r = tan(x);      TEST_ASSERT_EQUAL_DOUBLE( +1.000000   , r );
x = 7*pi/4;    r = tan(x);      TEST_ASSERT_EQUAL_DOUBLE( -1.000000   , r );
x = +0;        r = tan(x);      TEST_ASSERT_EQUAL_DOUBLE( 0.000000    , r );
x = -0;        r = tan(x);      TEST_ASSERT_EQUAL_DOUBLE( -0.000000   , r );
// x = INFINITY;  r = tan(x);   TEST_ASSERT_EQUAL_DOUBLE( -nan        , r );
}

void test_atan2(void) {
x = +1;        y = +1;   r = atan2(x, y);    TEST_ASSERT_EQUAL_DOUBLE( 0.785398    , r );
x = +1;        y = -1;   r = atan2(x, y);    TEST_ASSERT_EQUAL_DOUBLE( 2.356194    , r );
x = -1;        y = -1;   r = atan2(x, y);    TEST_ASSERT_EQUAL_DOUBLE( -2.356194   , r );
x = -1;        y = +1;   r = atan2(x, y);    TEST_ASSERT_EQUAL_DOUBLE( -0.785398   , r );
x = 0;         y = 0;    r = atan2(x, y);    TEST_ASSERT_EQUAL_DOUBLE( 0.000000    , r );
x = 7;         y = 0;    r = atan2(x, y);    TEST_ASSERT_EQUAL_DOUBLE( 1.570796    , r );
x = 7;         y = -0;   r = atan2(x, y);    TEST_ASSERT_EQUAL_DOUBLE( 1.570796    , r );
}

void test_atan(void) {
x = 1;         r = atan(x);     TEST_ASSERT_EQUAL_DOUBLE( 0.785398    , r );
// x = Inf;    r = atan(x);     TEST_ASSERT_EQUAL_DOUBLE( 1.570796    , r );
x = -0.0;      r = atan(x);     TEST_ASSERT_EQUAL_DOUBLE( -0.000000   , r );
x = +0.0;      r = atan(x);     TEST_ASSERT_EQUAL_DOUBLE( +0.000000   , r );
}

void test_asin(void) {
x =  1.0;      r = asin(x);     TEST_ASSERT_EQUAL_DOUBLE( +1.570796   , r );
x = -0.5;      r = asin(x);     TEST_ASSERT_EQUAL_DOUBLE( -0.523599   , r );
// x = 1.1;    r = asin(x);     TEST_ASSERT_EQUAL_DOUBLE( nan         , r );
}

void test_acos(void) {
x = -1;        r = acos(x);     TEST_ASSERT_EQUAL_DOUBLE( 3.141593    , r );
x = -0.9;      r = acos(x);     TEST_ASSERT_EQUAL_DOUBLE( 2.690565    , r );
x = 0.0;       r = acos(x);     TEST_ASSERT_EQUAL_DOUBLE( 1.570796    , r );
x = 0.5;       r = acos(x);     TEST_ASSERT_EQUAL_DOUBLE( 1.047198    , r );
x = 1;         r = acos(x);     TEST_ASSERT_EQUAL_DOUBLE( 0.000000    , r );
}

void test_trunc(void) {
x = +2.7;      r = trunc(x);    TEST_ASSERT_EQUAL_DOUBLE( +2.0        , r );
x = -2.7;      r = trunc(x);    TEST_ASSERT_EQUAL_DOUBLE( -2.0        , r );
x = -0.0;      r = trunc(x);    TEST_ASSERT_EQUAL_DOUBLE( -0.0        , r );
x = 2205.1969; r = trunc(x);    TEST_ASSERT_EQUAL_DOUBLE( 2205.000000 , r );
x = -270.8886; r = trunc(x);    TEST_ASSERT_EQUAL_DOUBLE( -270.000000 , r );
}

void test_floor(void) {
x = +2.7;      r = floor(x);    TEST_ASSERT_EQUAL_DOUBLE( +2.0        , r );
x = -2.7;      r = floor(x);    TEST_ASSERT_EQUAL_DOUBLE( -3.0        , r );
x = -0.0;      r = floor(x);    TEST_ASSERT_EQUAL_DOUBLE( -0.0        , r );
x = 2205.1969; r = floor(x);    TEST_ASSERT_EQUAL_DOUBLE( 2205.000000 , r );
x = -270.8886; r = floor(x);    TEST_ASSERT_EQUAL_DOUBLE( -271.000000 , r );
}

void test_ceil(void) {
x = +2.4;      r = ceil(x);     TEST_ASSERT_EQUAL_DOUBLE( +3.0        , r );
x = -2.4;      r = ceil(x);     TEST_ASSERT_EQUAL_DOUBLE( -2.0        , r );
x = -0.0;      r = ceil(x);     TEST_ASSERT_EQUAL_DOUBLE( -0.0        , r );
x = 2205.1969; r = ceil(x);     TEST_ASSERT_EQUAL_DOUBLE( 2206.000000 , r );
x = -270.8886; r = ceil(x);     TEST_ASSERT_EQUAL_DOUBLE( -270.000000 , r );
}

void test_fmod(void) {
x = +5.1;      y = +3.0; r = fmod(x, y);     TEST_ASSERT_EQUAL_DOUBLE( 2.1         , r );
x = -5.1;      y = +3.0; r = fmod(x, y);     TEST_ASSERT_EQUAL_DOUBLE( -2.1        , r );
x = +5.1;      y = -3.0; r = fmod(x, y);     TEST_ASSERT_EQUAL_DOUBLE( 2.1         , r );
x = -5.1;      y = -3.0; r = fmod(x, y);     TEST_ASSERT_EQUAL_DOUBLE( -2.1        , r );
x = +0.0;      y = 1.0;  r = fmod(x, y);     TEST_ASSERT_EQUAL_DOUBLE( 0.0         , r );
x = -0.0;      y = 1.0;  r = fmod(x, y);     TEST_ASSERT_EQUAL_DOUBLE( -0.0        , r );
// x = +5.1;   y = Inf;  r = fmod(x, y);     TEST_ASSERT_EQUAL_DOUBLE( 5.1         , r );
// x = +5.1;   y = 0;    r = fmod(x, y);     TEST_ASSERT_EQUAL_DOUBLE( nan         , r );
}

void test_fabs(void) {
x = +3;        r = fabs(x);     TEST_ASSERT_EQUAL_DOUBLE( 3.000000    , r );
x = -3;        r = fabs(x);     TEST_ASSERT_EQUAL_DOUBLE( 3.000000    , r );
x = -0;        r = fabs(x);     TEST_ASSERT_EQUAL_DOUBLE( 0.000000    , r );
x = 2205.1969; r = fabs(x);     TEST_ASSERT_EQUAL_DOUBLE( 2205.1969   , r );
x = -270.8886; r = fabs(x);     TEST_ASSERT_EQUAL_DOUBLE( 270.8886    , r );
}

int test_nanofloat(void) {
    //  Run unit tests.
    debug_flush();
    UNITY_BEGIN();

    RUN_TEST(test_aeabi); debug_flush();
    RUN_TEST(test_aeabi_fdiv); debug_flush();
    RUN_TEST(test_aeabi_ddiv); debug_flush();
    RUN_TEST(test_aeabi_dmul); debug_flush();

    RUN_TEST(test_sqrt); debug_flush();
    RUN_TEST(test_log); debug_flush();
    RUN_TEST(test_exp); debug_flush();
    RUN_TEST(test_log10); debug_flush();
    RUN_TEST(test_pow); debug_flush();
    RUN_TEST(test_ldexp); debug_flush();
    RUN_TEST(test_sin); debug_flush();
    RUN_TEST(test_cos); debug_flush();
    RUN_TEST(test_tan); debug_flush();
    RUN_TEST(test_atan2); debug_flush();
    RUN_TEST(test_atan); debug_flush();
    RUN_TEST(test_asin); debug_flush();
    RUN_TEST(test_acos); debug_flush();
    RUN_TEST(test_trunc); debug_flush();
    RUN_TEST(test_floor); debug_flush();
    RUN_TEST(test_ceil); debug_flush();
    RUN_TEST(test_fmod); debug_flush();
    RUN_TEST(test_fabs); debug_flush();

    int fails = UNITY_END(); debug_flush();

    //  Fetch the usage stats and display functions with no usage.
    uint16_t size = 0;
    uint8_t *float_usage = get_float_usage(&size);
    if (float_usage != NULL && size < 1000) {
        bool no_usage = false;
        uint16_t i = 0;        
        for (i = 1; i < size; i++) {
            if (float_usage[i] > 0) { continue; }
            if (!no_usage) {
                no_usage = true;
                debug_print("*** Functions not called: ");
            } else { debug_print(", "); }
            debug_printhex(i);
        }
        if (!no_usage) { debug_print("All functions called"); }
        debug_println("\nUsage: ");
        for (i = 1; i < size; i++) {
            if (float_usage[i] == 0) { continue; }
            debug_printhex(i); debug_print(" > "); debug_printhex(float_usage[i]); debug_print(" / ");
        }
        debug_println(""); debug_flush();
    }
    //  Crash and exit QEMU.
    rtc_awake_from_off(LSE);
    return fails;
}

void unity_output_char(int ch) {
    debug_write((uint8_t) ch);
}
#endif  //  UNIT_TEST
