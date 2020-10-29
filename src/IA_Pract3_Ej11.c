/*

 */

#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKL46Z4.h"
#include "fsl_debug_console.h"
/* TODO: insert other include files here. */

/* TODO: insert other definitions and declarations here. */

#define INCREMENTO  20000
#define T_MAXIMO   300000

typedef enum {
	APAGADOS,
	PARPADEANDO
} estados_MEF;

int main(void) {

	/* Init board hardware. */
	BOARD_InitBootPins();
	BOARD_InitBootClocks();
	BOARD_InitBootPeripherals();
	/* Init FSL debug console. */
	BOARD_InitDebugConsole();

	/*led VERDE*/
	SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;
	PORTD->PCR[5] |= PORT_PCR_MUX(1);
	PTD->PCOR |= 1 << 5;
	PTD->PDDR |= 1 << 5;

	/*ROJO*/
	SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;
	PORTE->PCR[29] |= PORT_PCR_MUX(1);
	PTE->PCOR |= 1 << 29;
	PTE->PDDR |= 1 << 29;

	/*SW1*/
	SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
	PORTC->PCR[3] |= PORT_PCR_MUX(1);
	PTC->PDDR &= ~(1 << 3);
	PORTC->PCR[3] |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;

	/*SW2*/
	SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
	PORTC->PCR[12] |= PORT_PCR_MUX(1);
	PTC->PDDR &= ~(1 << 12);
	PORTC->PCR[12] |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;

	/* Force the counter to be placed into memory. */
	volatile estados_MEF estado_actual = APAGADOS;
	volatile estados_MEF proximo_estado = APAGADOS;
	volatile static int i = 0;
	int T_BLINK= 110000;
	uint8_t pres_SW1 = 0;
	uint8_t pres_SW2 = 0;


	/* Enter an infinite loop, just incrementing a counter. */
	while (1) {

		switch (estado_actual) {
		case APAGADOS:
			PTD->PSOR |= (1 << 5);  // apagar led verde
			PTE->PSOR |= 1 << 29;   // apagar led rojo

			if (!(PTC->PDIR & (1 << 3)) && pres_SW1 == 0) { //pulsador presionado
				proximo_estado = PARPADEANDO;
				PTE->PCOR |= 1 << 29;   // encender led rojo
				i = T_BLINK;
				pres_SW1 = 1;
			}
			break;

		case PARPADEANDO:
			if (i > 0)
				i--;

			if (i == 0) {
				PTD->PTOR |= (1 << 5);
				PTE->PTOR |= (1 << 29);
				i = T_BLINK;

			}
			if (PTC->PDIR & (1 << 3)) {  // pulsador SW1 NO presionado
				pres_SW1 = 0;
			}

			if (!(PTC->PDIR & (1 << 3)) && pres_SW1 == 0) {
				pres_SW1 = 1;
				T_BLINK -= INCREMENTO;
				if (T_BLINK < 0)
					T_BLINK = 0;
			}

			if (PTC->PDIR & (1 << 12)) {  // pulsador SW2 NO presionado
				pres_SW2 = 0;
			}

			if (!(PTC->PDIR & (1 << 12)) && pres_SW2 == 0) {
				pres_SW2 = 1;
				T_BLINK += INCREMENTO;
				if (T_BLINK > T_MAXIMO)
					T_BLINK = T_MAXIMO;
			}

			break;
		} //fin del switch de estados

		if (estado_actual != proximo_estado) {
			estado_actual = proximo_estado;
		}
	}
	return 0;
}


