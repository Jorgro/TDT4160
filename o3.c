#include "o3.h"
#include "gpio.h"
#include "systick.h"

#define STATE_SEC 0
#define STATE_MIN 1
#define STATE_H 2
#define STATE_COUNT 3
#define STATE_ALARM 4





volatile gpio_map_t* gpio_p = (gpio_map_t*) GPIO_BASE;
volatile systick_map* systick_p = (systick_map*) SYSTICK_BASE;


/**************************************************************************//**
 * @brief Konverterer nummer til string
 * Konverterer et nummer mellom 0 og 99 til string
 *****************************************************************************/
void int_to_string(char *timestamp, unsigned int offset, int i) {
    if (i > 99) {
        timestamp[offset]   = '9';
        timestamp[offset+1] = '9';
        return;
    }

    while (i > 0) {
	    if (i >= 10) {
		    i -= 10;
		    timestamp[offset]++;

	    } else {
		    timestamp[offset+1] = '0' + i;
		    i=0;
	    }
    }
}

/**************************************************************************//**
 * @brief Konverterer 3 tall til en timestamp-string
 * timestamp-argumentet må være et array med plass til (minst) 7 elementer.
 * Det kan deklareres i funksjonen som kaller som "char timestamp[7];"
 * Kallet blir dermed:
 * char timestamp[7];
 * time_to_string(timestamp, h, m, s);
 *****************************************************************************/
void time_to_string(char *timestamp, int h, int m, int s) {
    timestamp[0] = '0';
    timestamp[1] = '0';
    timestamp[2] = '0';
    timestamp[3] = '0';
    timestamp[4] = '0';
    timestamp[5] = '0';
    timestamp[6] = '\0';

    int_to_string(timestamp, 0, h);
    int_to_string(timestamp, 2, m);
    int_to_string(timestamp, 4, s);
}


void setupPortPin(int pin, int port, int mode) {

	//Mode = 1 is INPUT, Mode = 0 is OUTPUT

	int doutclr_pin = 0b1 << pin;
	gpio_p->ports[port].DOUTCLR = doutclr_pin;

	if (pin >= 8) {

		pin = pin - 8;

	    if (mode) {
	    	gpio_p->ports[port].MODEH = (~(1111 << pin*4) & (gpio_p->ports[port].MODEH)) | (GPIO_MODE_INPUT << 4*pin);
	    } else {
	    	gpio_p->ports[port].MODEH = (~(1111 << pin*4) & (gpio_p->ports[port].MODEH)) | (GPIO_MODE_OUTPUT << 4*pin);
	    }

	} else {

	    if (mode) {
	    	gpio_p->ports[port].MODEL = (~(1111 << pin*4) & (gpio_p->ports[port].MODEL)) | (GPIO_MODE_INPUT << 4*pin);
	    } else {
	    	gpio_p->ports[port].MODEL = (~(1111 << pin*4) & (gpio_p->ports[port].MODEL)) | (GPIO_MODE_OUTPUT << 4*pin);
	    }

	}
}

void clearInterruptFlag(int pin) {
	gpio_p->IFC = gpio_p->IEN | 1 << pin;
}

void setupButton(int pin) {

	int real_pin = pin;

	if (pin >= 8) {
		pin = pin - 8;
		gpio_p->EXTIPSELH = (~(1111 << pin*4) & (gpio_p->EXTIPSELH)) | (GPIO_MODE_INPUT << 4*pin);
	} else {
		gpio_p->EXTIPSELL = (~(1111 << pin*4) & (gpio_p->EXTIPSELL)) | (GPIO_MODE_INPUT << 4*pin);
	}

	gpio_p->EXTIFALL = gpio_p->EXTIFALL | 1 << real_pin;
	gpio_p->IEN = gpio_p->IEN | 1 << real_pin;
	clearInterruptFlag(real_pin);

}




void turnOnLed() {
	gpio_p->ports[4].DOUTSET = 0b100;
}

void turnOffLed() {
	gpio_p->ports[4].DOUTCLR = 0b100;
}

//Noen nyttige globale variabler
static int state = STATE_SEC;
int h, m, s;
char timestamp[7];

void start() {
	systick_p->VAL = systick_p->LOAD;
	systick_p->CTRL = systick_p->CTRL | SysTick_CTRL_ENABLE_Msk;
}

void stop() {
	systick_p->CTRL = systick_p->CTRL & ~(SysTick_CTRL_ENABLE_Msk);
}

void update_display() {
    time_to_string(timestamp, h, m, s);
    lcd_write(timestamp);
}


void GPIO_ODD_IRQHandler(void) {
	switch(state) {
		case STATE_SEC: {
			++s;
			if (s == 60) {
				s = 0;
				++ m;
			}
			update_display();
		} break;
		case STATE_MIN: {
			++m;
			if (m == 60) {
				m = 0;
				++ h;
			}
			update_display();
		} break;
		case STATE_H: {
			++h;
			update_display();
		} break;
	}
	clearInterruptFlag(9);
}

void GPIO_EVEN_IRQHandler(void) {
	switch(state) {
		case STATE_SEC: {
			state = STATE_MIN;
		} break;
		case STATE_MIN: {
			state = STATE_H;
		} break;
		case STATE_H: {
			state = STATE_COUNT;
			start();
		} break;
		case STATE_ALARM: {
			state = STATE_SEC;
			turnOffLed();
		}
	}
	clearInterruptFlag(10);
}

void SysTick_Handler() {
	--s;
	if (s <= 0) {
		if (m <= 0) {
			if (h <= 0) {
				state = STATE_ALARM;
				turnOnLed();
				update_display();
				stop();
				return;
			}
			--h;
			m = 59;
		}
		--m;
		s = 59;
	}
	update_display();
}

int main(void) {
    init();

    // Skriv din kode her...

    //volatile uint32_t* ledpin = GPIO_BASE + GPIO_PORT_SIZE*GPIO_PORT_E + GPIO_PORT_DOUTSET;
    //Setup Pin 2 on Port 4 (LED)
    setupPortPin(2, 4, 0);
    setupPortPin(9, 1, 1);
    setupPortPin(10, 1, 1);
    //turnOnLed();
    //Port B pin 9 og 10 er buttons
    setupButton(9);
    setupButton(10);
    //time_to_string(timestamp, 1, 1, 1);
    //lcd_write(timestamp);
    systick_p->LOAD = FREQUENCY;
    systick_p->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk;
    //start();
    while (1);
    return 0;
}

