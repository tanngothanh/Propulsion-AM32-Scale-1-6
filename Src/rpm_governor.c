#include "rpm_governor.h"

uint8_t rpm_governor_calc_crc8_atm(const uint8_t *data, uint8_t len) {
    uint8_t crc = 0x00;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x07;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

bool rpm_governor_validate_config(const uint8_t *block) {
    if (!block) {
        return false;
    }
    uint8_t expected_crc = rpm_governor_calc_crc8_atm(block, 7);
    return (block[7] == expected_crc);
}

void rpm_governor_failsafe_reset(rpm_governor_failsafe_state_t *state, uint32_t current_zc) {
    if (!state) {
        return;
    }
    state->last_zero_crosses = current_zc;
    state->loss_timer_ms = 0;
    state->latched_fault = 0;
}
