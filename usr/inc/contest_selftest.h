#ifndef CONTEST_SELFTEST_H_
#define CONTEST_SELFTEST_H_

#include <stdint.h>

#define CONTEST_SELFTEST_FAIL_STATS        (1UL << 0)
#define CONTEST_SELFTEST_FAIL_FREQUENCY    (1UL << 1)
#define CONTEST_SELFTEST_FAIL_PHASE        (1UL << 2)
#define CONTEST_SELFTEST_FAIL_GOERTZEL     (1UL << 3)
#define CONTEST_SELFTEST_FAIL_CALIBRATION  (1UL << 4)
#define CONTEST_SELFTEST_FAIL_PID          (1UL << 5)
#define CONTEST_SELFTEST_FAIL_CRC          (1UL << 6)
#define CONTEST_SELFTEST_FAIL_PROTOCOL     (1UL << 7)

uint32_t ContestSelfTest_Run(void);

#endif /* CONTEST_SELFTEST_H_ */
