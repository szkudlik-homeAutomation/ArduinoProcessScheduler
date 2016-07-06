#ifndef SCHEDULER_PROCESS_INCLUDES_H
#define SCHEDULER_PROCESS_INCLUDES_H

#include <Arduino.h>
#include <RingBuf.h>
#include "Config.h"


class Scheduler;
class Process;

typedef enum ProcessWarning
{
    WARNING_PROC_OVERSCHEDULED = 0,

#ifdef _PROCESS_TIMEOUT_INTERRUPTS
    WARNING_PROC_TIMED_OUT
#endif
} ProcessWarning;

// PICK INT VALUES PEOPLE UNLIKLEY TO USE
#define LONGJMP_ISR_CODE -1000
#define LONGJMP_YIELD_CODE -1001

#if defined(ARDUINO_ARCH_AVR)
    #include <setjmp.h>
    #include <util/atomic.h>
    #define ATOMIC_START ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    #define ATOMIC_END }

    #include <avr/sleep.h>
    #define HALT_PROCESSOR() \
            do { noInterrupts(); sleep_enable(); sleep_cpu(); } while(0)

    #define ENABLE_SCHEDULER_ISR() \
            do { OCR0A = 0xAA; TIMSK0 |= _BV(OCIE0A); } while(0)


    #define DISABLE_SCHEDULER_ISR() \
            do { TIMSK0 &= ~_BV(OCIE0A); } while(0)


#elif defined(ARDUINO_ARCH_ESP8266)
    #ifndef __STRINGIFY
    #define __STRINGIFY(a) #a
    #endif

    #ifndef xt_rsil
        #define xt_rsil(level) (__extension__({uint32_t state; __asm__ __volatile__("rsil %0," __STRINGIFY(level) : "=a" (state)); state;}))
    #endif

    #ifndef xt_wsr_ps
        #define xt_wsr_ps(state)  __asm__ __volatile__("wsr %0,ps; isync" :: "a" (state) : "memory")
    #endif

    #define ATOMIC_START do { uint32_t _savedIS = xt_rsil(15) ;
    #define ATOMIC_END xt_wsr_ps(_savedIS) ;} while(0);

    #define HALT_PROCESSOR() \
            ESP.deepSleep(0)

    // Not supported on ESP8266
    #define ENABLE_SCHEDULER_ISR()
    #define DISABLE_SCHEDULER_ISR()

#else
    #error “This library only supports AVR and ESP8266 Boards.”
#endif


#ifdef _MICROS_PRECISION
    #define TIMESTAMP() micros()
#else
    #define TIMESTAMP() millis()
#endif

#if defined(_PROCESS_EXCEPTION_HANDLING) && defined(ARDUINO_ARCH_ESP8266)
    #error "'_PROCESS_EXCEPTION_HANDLING' is not supported on the ESP8266."
#endif


#if defined(_PROCESS_TIMEOUT_INTERRUPTS) && !defined(_PROCESS_EXCEPTION_HANDLING)
    #error "'_PROCESS_TIMEOUT_INTERRUPTS' requires enabling `_PROCESS_EXCEPTION_HANDLING`"
#endif
#endif