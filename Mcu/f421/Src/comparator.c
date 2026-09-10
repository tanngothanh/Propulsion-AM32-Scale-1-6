/*
 * comparator.c
 *
 *  Created on: Sep. 26, 2020
 *      Author: Alka
 */

#include "comparator.h"
#include "targets.h"
#include "common.h"

uint8_t getCompOutputLevel() { return CMP->ctrlsts_bit.cmpvalue; }

void maskPhaseInterrupts()
{
    EXINT->inten &= ~EXTI_LINE;
    EXINT->intsts = EXTI_LINE;
}

void enableCompInterrupts() { EXINT->inten |= EXTI_LINE; }

void changeCompInput()
{
    if (step == 1 || step == 4) { // c floating
        CMP->ctrlsts = PHASE_C_COMP;
    }
    if (step == 2 || step == 5) { // a floating
        CMP->ctrlsts = PHASE_A_COMP;
    }
    if (step == 3 || step == 6) { // b floating
        CMP->ctrlsts = PHASE_B_COMP;
    }
    if (average_interval > 50 && average_interval < 400) {
        CMP->ctrlsts = CMP->ctrlsts & ~(1 << 2); // High speed mode (<50ns) ONLY at confirmed high RPM
    } else {
        CMP->ctrlsts = CMP->ctrlsts | (1 << 2);  // Medium speed mode at standstill (0), startup, and low RPM
    }
	EXINT->polcfg1 = !rising << 21;
    EXINT->polcfg2 = rising << 21;
}
