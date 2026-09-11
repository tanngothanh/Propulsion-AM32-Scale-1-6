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

bool rpm_governor_failsafe_update(
    rpm_governor_failsafe_state_t *state,
    uint32_t current_zc,
    uint16_t dshot_cmd,
    uint8_t timeout_ms,
    bool governor_active,
    bool running
) {
    if (!state) {
        return false;
    }
    if (dshot_cmd < DSHOT_MIN_THROTTLE) {
        state->latched_fault = 0;
        state->loss_timer_ms = 0;
        state->last_zero_crosses = current_zc;
        return false;
    }
    if (state->latched_fault) {
        state->last_zero_crosses = current_zc;
        return true;
    }
    if (!governor_active || !running || timeout_ms == 0) {
        state->loss_timer_ms = 0;
        state->last_zero_crosses = current_zc;
        return false;
    }
    if (current_zc != state->last_zero_crosses) {
        state->last_zero_crosses = current_zc;
        state->loss_timer_ms = 0;
        return false;
    }
    if (state->loss_timer_ms < 0xFFFF) {
        state->loss_timer_ms++;
    }
    if (state->loss_timer_ms >= timeout_ms) {
        state->latched_fault = 1;
        return true;
    }
    return false;
}

__attribute__((noinline, used)) uint32_t rpm_governor_calc_com_time(uint32_t target_rpm, uint8_t poles) {
    if (target_rpm == 0 || poles < 2) {
        return 0;
    }
    return 60000000UL / target_rpm / (poles / 2);
}

uint16_t rpm_governor_calc_com_time_u16(uint32_t target_rpm, uint8_t poles) {
    uint32_t com_time = rpm_governor_calc_com_time(target_rpm, poles);
    if (com_time > 0xFFFF) {
        return 0xFFFF;
    }
    return (uint16_t)com_time;
}
