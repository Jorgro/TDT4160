/* En rask måte å unngå header recursion på er å sjekke om verdi, f.eks. 'O3_H',
   er definert. Hvis ikke, definer 'O3_H' og deretter innholdet av headeren
   (merk endif på bunnen). Nå kan headeren inkluderes så mange ganger vi vil
   uten at det blir noen problemer. */
#ifndef O3_H
#define O3_H

// Type-definisjoner fra std-bibliotekene
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

// Type-aliaser
typedef uint32_t word;
typedef uint8_t  byte;

#include "gpio.h"
// Prototyper for bibliotekfunksjoner
void init(void);
void lcd_write(char* string);
void int_to_string(char *timestamp, unsigned int offset, int i);
void time_to_string(char *timestamp, int h, int m, int s);
void setupPortPin(int pin, int port,int mode);
void turnOnLed();
void setupButton(int pin);
void GPIO_EVEN_IRQHandler(void);
void GPIO_EVEN_IRQHandler(void);
// Prototyper
// legg prototyper for dine funksjoner her

#endif
