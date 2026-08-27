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

uint32_t rpm_governor_map_dshot(uint16_t dshot_val, uint32_t rpm_min, uint32_t rpm_max) {
    if (dshot_val == 0 || dshot_val < DSHOT_MIN_THROTTLE) {
        return 0;
    }
    if (dshot_val >= DSHOT_MAX_THROTTLE) {
        return rpm_max;
    }
    return rpm_min + ((uint32_t)(dshot_val - DSHOT_MIN_THROTTLE) * (rpm_max - rpm_min) + 999u) / 1999u;
}

uint32_t rpm_governor_calc_com_time(uint32_t target_rpm, uint8_t poles) {
    if (target_rpm == 0 || poles < 2) {
        return 0;
    }
    return 60000000UL / target_rpm / (poles / 2);
}

bool rpm_governor_check_current_trip(uint16_t actual_current_cA, uint16_t max_current_cA) {
    return (actual_current_cA >= max_current_cA);
}
