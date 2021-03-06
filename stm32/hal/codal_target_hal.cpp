//  HAL for STM32 Blue Pill. Based on https://github.com/mmoskal/codal-generic-f103re/blob/master/source/codal_target_hal.cpp
#include <string.h>
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/rtc.h>
#include <libopencm3/stm32/pwr.h>
#include <newlib-force.h>  //  Force newlib to be included for build
#include <nano-float.h>    //  Force nano-float to be included for build
#include <bootloader/bootloader.h>
#include <cocoos.h>
#include <bluepill.h>
#include <logger.h>
#include "platform_includes.h"

//  Blue Pill Memory Layout: ~/.platformio/packages/framework-libopencm3/lib/stm32/f1/stm32f103x8.ld
//  RAM Layout: [Start of RAM] [Data] [BSS] [Codal Heap] grows--> (empty) <--grows [Stack] [End of RAM]

extern PROCESSOR_WORD_TYPE _ebss;  //  End of BSS.
PROCESSOR_WORD_TYPE codal_heap_start = (PROCESSOR_WORD_TYPE)(&_ebss);  //  Start of heap is the end of BSS.

// extern "C" void init_irqs();
extern "C" void test_codal();
static void os_preschedule(void);
static void os_schedule(void);
void stm32bluepill_dmesg_flush();
static bool initialised = false;
static void (*tick_callback)() = NULL;
static void (*alarm_callback)() = NULL;
static int (*bootloader_callback)() = NULL;
static volatile int poll_status = 0;
static volatile int prev_poll_status = 0;
static volatile uint32_t poll_stop = 0;  //  Elaspsed time since startup that we should stop polling.
#define MAX_BURST_POLL 10  //  If there is USB activity, poll USB requests for up to 10 times continuously.

static void poll_bootloader() {
    //  If bootloader is running in background, call it to handle USB requests.
    if (!bootloader_callback) { return; }
    //  If we have received any USB request, continue polling a few times.  That's because according to the USB 2.0 specs,
    //  we must return the response for the Set Address request within 50 ms.  USB Enumeration requests need to be
    //  handled ASAP or Windows / Mac will report the Blue Pill as an unknown device.
    poll_status = bootloader_callback();
    prev_poll_status = poll_status;
    if (poll_status > 0) { debug_print("u{ "); }
    while (poll_status > 0) {  //  If we receive any USB requests,,,
        poll_status = 0;       //  Continue polling a few times for subsequent USB requests.
        for (uint16_t i = 0; i < MAX_BURST_POLL; i++) {
            //  We can't poll too many times because this is a timer tick, it should terminate in 1 millisecond.
            poll_status = poll_status | bootloader_callback();
        }
    }
    if (prev_poll_status > 0) { debug_print("} "); }
}

static void timer_tick() {
    //  This is called every millisecond.  
    //  If bootloader is running in background, call it to handle USB requests.
    poll_bootloader();
    //  If Codal Timer exists, update the timer.
    if (tick_callback) { tick_callback(); }
    //  Call cocoOS at every tick.
    os_tick();
}

static void timer_alarm() {
    //  This is called when the Real-Time Clock alarm is triggered.
#ifdef NOTUSED
    //  If we have received USB requests, handle them now.
    if (bootloader_status() > 0) {
        debug_print(";");
        poll_bootloader();
    }
#endif  //  NOTUSED
    //  If Codal Timer exists, update the timer.
    if (alarm_callback) { alarm_callback(); }
    else { if (millis() < 200) { debug_print("a? "); } }
}

void target_init(void) {
    //  Blue Pill specific initialisation...
    if (initialised) { return; }  //  Already initialised, skip.
    initialised = true;

    //  Init the platform, cocoOS and create any system objects.
    platform_setup();  //  STM32 platform setup.
    os_init();         //  Init cocoOS before creating any multitasking objects.
    // TODO: init_irqs();  //  Init the interrupt routines.

    //  Start the STM32 timer to generate millisecond-ticks for measuring elapsed time.
    platform_start_timer(timer_tick, timer_alarm);

    //  Display the dmesg log when idle.
    //  TODO: codal_dmesg_set_flush_fn(target_dmesg_flush);

    //  TODO: Seed our random number generator
    //  seedRandom();
}

#define FLUSH_INTERVAL 500  //  Flush the logs every 0.5 seconds.
static volatile uint32_t last_flush = 0;

void target_wait_for_event() {
    //  Run background tasks and sleep until an event happens e.g. alarm.
  	//  debug_println("----target_wait_for_event"); // 
    if (!initialised) { return; }  //  If not initialised, quit.

    //  Run a cocoOS task if any.  Must be called only when all the tasks have been created.
    ////if (!os_running()) { os_preschedule(); }  //  Start the cocoOS scheduler if not started.
    ////os_schedule();  

#ifdef NOTUSED  //  Flush doesn't work because this is called from an interrupt service routine.
    //  Flush the debug log buffers once in a while.
    if ((last_flush + FLUSH_INTERVAL) >= millis()) {
        last_flush = millis();
        debug_flush();
        target_dmesg_flush();
    }
#endif  //  NOTUSED
    __asm("wfe");  //  Allow CPU to go to sleep.
}

void target_wait(uint32_t milliseconds) {
    //  Wait for the specified number of milliseconds.
    if (milliseconds <= 0) { return; }
    if (!initialised) { return; }  //  If not initialised, quit.
    debug_print("wt <"); debug_print_unsigned(milliseconds / 1000);
    uint32_t end = millis() + milliseconds;
    for (;;) {
        if (millis() >= end) { break; }
        if (os_running()) { os_schedule(); }  //  Schedule a cocoOS task to run.
        __asm("wfe");  //  Allow CPU to go to sleep.
    }
    debug_print("> ");
}

void target_wait_us(unsigned long microseconds) {
    //  Wait for the specified number of microseconds.
    return target_wait(microseconds / 1000);
}

void target_reset() {
	//  Restart the device.
  	debug_println("----target_reset"); debug_flush();
    scb_reset_system();
}

#if DEVICE_DMESG_BUFFER_SIZE > 0
//  TODO: Sync with lib/codal-core/inc/core/CodalDmesg.h
struct CodalLogStore
{
    uint32_t ptr;
    char buffer[DEVICE_DMESG_BUFFER_SIZE];
};
extern struct CodalLogStore codalLogStore;
#endif  //  DEVICE_DMESG_BUFFER_SIZE

void target_dmesg_flush() {
#if DEVICE_DMESG_BUFFER_SIZE > 0
    //  Flush the dmesg log to the debug console.
    if (codalLogStore.ptr > 0 && initialised) {
        for (uint32_t i = 0; i < codalLogStore.ptr; i++) {
            debug_print((uint8_t) codalLogStore.buffer[i]);
        }
        codalLogStore.ptr = 0;
        debug_flush();
    }
#endif  //  DEVICE_DMESG_BUFFER_SIZE
}

// From pxt-common-packages/libs/base/pxtbase.h:
// #define PXT_IN_ISR() (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk)

uint32_t target_in_isr(void) {
    //  Return true if CPU is in ISR now.
    return SCB_ICSR & SCB_ICSR_VECTACTIVE;
}

#define SET_BIT(var, bit)   { var |= bit; }   //  Set the specified bit of var to 1, e.g. SET_BIT(SCB_SCR, SCB_SCR_SLEEPDEEP) sets bit SCB_SCR_SLEEPDEEP of SCB_SCR to 1.
#define CLEAR_BIT(var, bit) { var &= ~bit; }  //  Set the specified bit of var to 0, e.g. CLEAR_BIT(SCB_SCR, SCB_SCR_SLEEPDEEP) sets bit SCB_SCR_SLEEPDEEP of SCB_SCR to 0.

void target_enter_sleep_mode(void) {
    //  To enter Sleep Now Mode: WFI (Wait for Interrupt) or WFE (Wait for Event) while:
    //  – SLEEPDEEP = 0 and
    //  – SLEEPONEXIT = 0 
    //  Assume caller has set RTC Wakeup Alarm.
    CLEAR_BIT(SCB_SCR, SCB_SCR_SLEEPDEEP);    //  Clear SLEEPDEEP bit of Cortex System Control Register.
    CLEAR_BIT(SCB_SCR, SCB_SCR_SLEEPONEXIT);  //  Clear SLEEPONEXIT bit of Cortex System Control Register.
    __asm("wfi");  //  Wait for interrupt from RTC Alarm.
}

void target_enter_deep_sleep_stop_mode(void) {
    //  The Stop mode is based on the Cortex®-M3 deepsleep mode combined with peripheral
    //  clock gating. The voltage regulator can be configured either in normal or low-power mode.
    //  In Stop mode, all clocks in the 1.8 V domain are stopped, the PLL, the HSI and the HSE RC
    //  oscillators are disabled. SRAM and register contents are preserved.
    //  In the Stop mode, all I/O pins keep the same state as in the Run mode.
    //  To enter Stop Mode: 
    //  WFI (Wait for Interrupt) or WFE (Wait for Event) while:
    //  – Set SLEEPDEEP bit in Cortex®-M3 System Control register
    //  – Clear PDDS bit in Power Control register (PWR_CR)
    //  – Select the voltage regulator mode by configuring LPDS bit in PWR_CR
    //  Note: To enter Stop mode, all EXTI Line pending bits (in Pending register
    //  (EXTI_PR)), all peripheral interrupt pending bits, and RTC Alarm flag must
    //  be reset. Otherwise, the Stop mode entry procedure is ignored and
    //  program execution continues. 
    //  Assume caller has set RTC Wakeup Alarm.

    pwr_set_stop_mode();   //  Clear PWR_CR_PDDS.
    pwr_voltage_regulator_low_power_in_stop();  //  Switch voltage regulator to low power mode.
    SET_BIT(SCB_SCR, SCB_SCR_SLEEPDEEP);        //  Set SLEEPDEEP bit of Cortex System Control Register.
    __asm("wfi");  //  Wait for interrupt from RTC Alarm.
}

void target_enter_deep_sleep_standby_mode(void) {
    //  The Standby mode allows to achieve the lowest power consumption. It is based on the
    //  Cortex®-M3 deepsleep mode, with the voltage regulator disabled. The 1.8 V domain is
    //  consequently powered off. The PLL, the HSI oscillator and the HSE oscillator are also
    //  switched off. SRAM and register contents are lost except for registers in the Backup domain
    //  and Standby circuitry.  To enter Standby Mode: 
    //  WFI (Wait for Interrupt) or WFE (Wait for Event) while:
    //  – Set SLEEPDEEP in Cortex®-M3 System Control register
    //  – Set PDDS bit in Power Control register (PWR_CR)
    //  – Clear WUF bit in Power Control/Status register (PWR_CSR)
    //  – No interrupt (for WFI) or event (for WFI) is pending
    //  Assume caller has set RTC Wakeup Alarm.

    pwr_set_standby_mode();   //  Set PWR_CR_PDDS.
    pwr_clear_wakeup_flag();  //  Clear WUF.    
    SET_BIT(SCB_SCR, SCB_SCR_SLEEPDEEP);  //  Set SLEEPDEEP bit of Cortex System Control Register.
    __asm("wfi");  //  Wait for interrupt from RTC Alarm.
}

void target_enable_irq() {
    //  Enable interrupts.
  	//  debug_println("----target_enable_irq"); debug_flush();
	cm_enable_interrupts();
}

void target_disable_irq() {
    //  Disable interrupts.
  	//  debug_println("----target_disable_irq"); debug_flush();
	cm_disable_interrupts();
}

int target_seed_random(uint32_t rand) {
    //  TODO: return codal::seed_random(rand);
    debug_println("----target_seed_random");
    return 0;
}

int target_random(int max) {
    //  TODO: return codal::random(max);
    debug_println("----target_random");
    return 0;
}

/*
    The unique device identifier is ideally suited:
        * for use as serial numbers (for example USB string serial numbers or other end applications)
        * for use as security keys in order to increase the security of code in Flash memory while using and combining this unique ID with software cryptographic primitives and protocols before programming the internal Flash memory
        * to activate secure boot processes, etc.
    The 96-bit unique device identifier provides a reference number which is unique for any
    device and in any context. These bits can never be altered by the user.
    The 96-bit unique device identifier can also be read in single bytes/half-words/words in different ways and then be concatenated using a custom algorithm.
*/
#define STM32_UUID ((uint32_t *)0x1FFF7A10)
uint32_t target_get_serial() {
    //  TODO: Use bootloader ID.
    // uuid[1] is the wafer number plus the lot number, need to check the uniqueness of this...
    debug_println("----target_get_serial");
    return (uint32_t)STM32_UUID[1];
}

extern "C" void assert_failed(uint8_t* file, uint32_t line) {
    debug_print("*** assert failed: ");  // debug_print(file); 
    debug_print((size_t) line);
    debug_println(""); debug_flush();
    target_panic(920);
}

void target_panic(int statusCode) {
	//  Stop due to an error.
    target_disable_irq();
	debug_print("*****target_panic ");
	debug_println((int) statusCode);
	debug_flush();
    //  DMESG("*** CODAL PANIC : [%d]", statusCode);
	for (;;) {
        __asm("wfe");  //  Allow CPU to go to sleep.
    }  //  Loop forever.
}

/**
 *  Thread Context for an ARM Cortex core.
 *
 * This is probably overkill, but the ARMCC compiler uses a lot register optimisation
 * in its calling conventions, so better safe than sorry!
 */
struct PROCESSOR_TCB {
    uint32_t R0;
    uint32_t R1;
    uint32_t R2;
    uint32_t R3;
    uint32_t R4;
    uint32_t R5;
    uint32_t R6;
    uint32_t R7;
    uint32_t R8;
    uint32_t R9;
    uint32_t R10;
    uint32_t R11;
    uint32_t R12;
    uint32_t SP;
    uint32_t LR;
    uint32_t stack_base;
};

PROCESSOR_WORD_TYPE fiber_initial_stack_base() {
    return (PROCESSOR_WORD_TYPE) DEVICE_STACK_BASE;
}

void *tcb_allocate() {
    return (void *)malloc(sizeof(PROCESSOR_TCB));
}

/**
 * Configures the link register of the given tcb to have the value function.
 *
 * @param tcb The tcb to modify
 * @param function the function the link register should point to.
 */
void tcb_configure_lr(void *tcb, PROCESSOR_WORD_TYPE function) {
    PROCESSOR_TCB *tcbPointer = (PROCESSOR_TCB *)tcb;
    tcbPointer->LR = function;
}

/**
 * Configures the link register of the given tcb to have the value function.
 *
 * @param tcb The tcb to modify
 * @param function the function the link register should point to.
 */
void tcb_configure_sp(void *tcb, PROCESSOR_WORD_TYPE sp) {
    PROCESSOR_TCB *tcbPointer = (PROCESSOR_TCB *)tcb;
    tcbPointer->SP = sp;
}

void tcb_configure_stack_base(void *tcb, PROCESSOR_WORD_TYPE stack_base) {
    PROCESSOR_TCB *tcbPointer = (PROCESSOR_TCB *)tcb;
    tcbPointer->stack_base = stack_base;
}

PROCESSOR_WORD_TYPE tcb_get_stack_base(void *tcb) {
    PROCESSOR_TCB *tcbPointer = (PROCESSOR_TCB *)tcb;
    return tcbPointer->stack_base;
}

register unsigned int _sp __asm("sp");

PROCESSOR_WORD_TYPE get_current_sp() {
    //  Get the current Stack Pointer.
    return _sp;  //  Formerly: return __get_MSP();
}

PROCESSOR_WORD_TYPE tcb_get_sp(void *tcb) {
    PROCESSOR_TCB *tcbPointer = (PROCESSOR_TCB *)tcb;
    return tcbPointer->SP;
}

void tcb_configure_args(void *tcb, PROCESSOR_WORD_TYPE ep, PROCESSOR_WORD_TYPE cp,
                        PROCESSOR_WORD_TYPE pm) {
    PROCESSOR_TCB *tcbPointer = (PROCESSOR_TCB *)tcb;
    tcbPointer->R0 = (uint32_t)ep;
    tcbPointer->R1 = (uint32_t)cp;
    tcbPointer->R2 = (uint32_t)pm;
}

void test_codal() {
	PROCESSOR_WORD_TYPE start = (PROCESSOR_WORD_TYPE)(codal_heap_start); 
	PROCESSOR_WORD_TYPE end = (PROCESSOR_WORD_TYPE)(DEVICE_STACK_BASE) - (PROCESSOR_WORD_TYPE)(DEVICE_STACK_SIZE);
	PROCESSOR_WORD_TYPE size = end - start;
	debug_print("heap start: "); debug_printhex_unsigned((size_t) start);
	debug_print(", end: ");      debug_printhex_unsigned((size_t) end);
	debug_print(", size: ");     debug_print((size_t) (size >> 10));
	debug_print(" KB, stack used: "); debug_print((size_t) 
        ((PROCESSOR_WORD_TYPE)(DEVICE_STACK_BASE) - get_current_sp()) >> 10);
    debug_println(" KB"); debug_flush();
}

//  Handle exit.  From https://arobenko.gitbooks.io/bare_metal_cpp/content/compiler_output/static.html.
extern "C" {
    void* __dso_handle = nullptr;
    void _fini(void) { }
    int __aeabi_atexit( 
        void *object, 
        void (*destructor)(void *), 
        void *dso_handle) 
    { 
        static_cast<void>(object); 
        static_cast<void>(destructor); 
        static_cast<void>(dso_handle); 
        return 0; 
    }
}

void target_set_tick_callback(void (*tick_callback0)()) {
    //  The callback is normally set to CMTimer::tick_callback(), which calls Timer::trigger() to resume suspended tasks.
    tick_callback = tick_callback0;
}

void target_set_alarm_callback(void (*alarm_callback0)()) {
    //  The callback is normally set to CMTimer::alarm_callback(), which calls Timer::trigger() to resume suspended tasks.
    alarm_callback = alarm_callback0;
}

void target_set_bootloader_callback(int (*bootloader_callback0)()) {
    //  The bootloader callback is called every 1 millisec to handle USB requests in the background.
    bootloader_callback = bootloader_callback0;
}

void target_enable_debug(void) {
    //  Allow display of debug messages in development devices. NOTE: This will hang if no debugger is attached.
    enable_debug();   
}

void target_disable_debug(void) {
    //  Disable display of debug messages.  For use in production devices.
    disable_debug();  
}

//  Schedule a cocoOS task for running.  Copied from https://github.com/cocoOS/cocoOS/blob/master/src/os_kernel.c
extern uint8_t running_tid, last_running_task, running;  //  System flags. 

static void os_preschedule(void) {
    //  Must be called once before os_schedule().
    if (os_running()) { return; }  //  Already running.
    running = 1;
    os_enable_interrupts();
}

static void os_schedule( void ) {
    //  Call this to schedule a task.
    if (!os_running()) { return; }  //  Don't schedule if cocoOS scheduler is not started.
    running_tid = NO_TID;
#ifdef ROUND_ROBIN
    /* Find next ready task */
    running_tid = os_task_next_ready_task();
#else
    /* Find the highest prio task ready to run */
    running_tid = os_task_highest_prio_ready_task();   
#endif    
    if ( running_tid != NO_TID ) {
        os_task_run();
    }
    else {
        os_cbkSleep();
    }
}

#ifdef NOTUSED
    //  cocoOS context and objects.
    static struct USBContext {} context;
    static Sem_t usb_semaphore;

    static void usb_task(void) {
        //  cocoOS task that runs forever waiting for the USB semaphore to be signalled by the tick interrupts.
        task_open();  //  Start of the task. Must be matched with task_close().
        for (;;) {
            sem_wait(usb_semaphore);       //  Wait for the semaphore to be signalled.
            if (!bootloader_callback) { continue; }
            //  debug_print(" u "); ////
            bootloader_callback();         //  Trigger the Bootloader background processing.
        }
        task_close();  //  End of the task. Should not come here.
    }

    //  Create a semaphore to signal the USB Task on a tick interrupt.
    usb_semaphore = sem_bin_create(0);  //  Binary Semaphore: Will wait until signalled.

    //  Create the cocoOS USB Task.
    task_create(
        usb_task,   //  Task will run this function.
        &context,     //  task_get_data() will be set to the context object.
        30,           //  Priority 30
        NULL, 0, 0);
#endif  //  NOTUSED
