#ifndef RPM_GOVERNOR_H
#define RPM_GOVERNOR_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DSHOT_MIN_THROTTLE 48u
#define DSHOT_MAX_THROTTLE 2047u
#define DC_BUS_CURRENT_TRIP_CA 2800u /* 28.00 A */

typedef struct {
    uint8_t rpm_mode_and_min; /* bit7: mode (0=OFF, 1=ON), bits0-6: RPM_MIN/100 (10..90) */
    uint8_t rpm_max_div_100;  /* RPM_MAX/100 (10..90) */
    uint8_t kp_raw;           /* raw Kp */
    uint8_t ki_raw;           /* raw Ki */
    uint8_t slew_div_250;     /* slew rate / 250 RPM/s */
    uint8_t delta_v_div_0_1;  /* delta V / 0.1 V */
    uint8_t erpm_loss_ms;     /* eRPM loss timeout in ms */
    uint8_t crc8_atm;         /* CRC-8/ATM over the first 7 bytes */
} __attribute__((packed)) rpm_governor_config_block_t;

typedef struct {
    volatile uint32_t last_zero_crosses;
    volatile uint16_t loss_timer_ms;
    volatile uint8_t latched_fault;
} rpm_governor_failsafe_state_t;

uint8_t rpm_governor_calc_crc8_atm(const uint8_t *data, uint8_t len);
bool rpm_governor_validate_config(const uint8_t *block);
uint32_t rpm_governor_map_dshot(uint16_t dshot_val, uint32_t rpm_min, uint32_t rpm_max);
uint32_t rpm_governor_calc_com_time(uint32_t target_rpm, uint8_t poles);
uint16_t rpm_governor_calc_com_time_u16(uint32_t target_rpm, uint8_t poles);
bool rpm_governor_check_current_trip(uint16_t actual_current_cA, uint16_t max_current_cA);
void rpm_governor_failsafe_reset(rpm_governor_failsafe_state_t *state, uint32_t current_zc);
bool rpm_governor_failsafe_update(
    rpm_governor_failsafe_state_t *state,
    uint32_t current_zc,
    uint16_t dshot_cmd,
    uint8_t timeout_ms,
    bool governor_active,
    bool running
);

#ifdef __cplusplus
}
#endif

#endif /* RPM_GOVERNOR_H */
