"""Numerical oracle for the MSPM0 contest core self-test vectors.

This validates the expected values used by contest_selftest.c.  It does not
replace the on-target test, which executes the actual C implementation.
"""

import math
import statistics


SAMPLE_COUNT = 1024
SAMPLE_RATE_HZ = 12800.0
SIGNAL_HZ = 50.0
PHASE_DEG = 30.0
OFFSET = 2048.0
AMPLITUDE_A = 1000.0
AMPLITUDE_B = 800.0


def make_signal(amplitude: float, phase_deg: float) -> list[int]:
    phase = math.radians(phase_deg)
    return [
        int(
            OFFSET
            + amplitude
            * math.sin(2.0 * math.pi * SIGNAL_HZ * i / SAMPLE_RATE_HZ + phase)
            + 0.5
        )
        for i in range(SAMPLE_COUNT)
    ]


def estimate_frequency(data: list[int]) -> float:
    mean = statistics.fmean(data)
    crossings: list[float] = []
    for i in range(1, len(data)):
        previous = data[i - 1] - mean
        current = data[i] - mean
        if previous <= 0.0 < current:
            fraction = -previous / (current - previous)
            crossings.append((i - 1) + fraction)
    return SAMPLE_RATE_HZ * (len(crossings) - 1) / (crossings[-1] - crossings[0])


def estimate_phase(a: list[int], b: list[int]) -> float:
    mean_a = statistics.fmean(a)
    mean_b = statistics.fmean(b)
    real_a = imag_a = real_b = imag_b = 0.0
    omega = 2.0 * math.pi * SIGNAL_HZ / SAMPLE_RATE_HZ
    for i, (sample_a, sample_b) in enumerate(zip(a, b)):
        cosine = math.cos(omega * i)
        sine = math.sin(omega * i)
        real_a += (sample_a - mean_a) * cosine
        imag_a -= (sample_a - mean_a) * sine
        real_b += (sample_b - mean_b) * cosine
        imag_b -= (sample_b - mean_b) * sine
    phase = math.degrees(math.atan2(imag_b, real_b) - math.atan2(imag_a, real_a))
    return (phase + 180.0) % 360.0 - 180.0


def goertzel_magnitude(data: list[int]) -> float:
    mean = statistics.fmean(data)
    omega = 2.0 * math.pi * SIGNAL_HZ / SAMPLE_RATE_HZ
    coefficient = 2.0 * math.cos(omega)
    state1 = state2 = 0.0
    for sample in data:
        state0 = (sample - mean) + coefficient * state1 - state2
        state2 = state1
        state1 = state0
    power = state1 * state1 + state2 * state2 - coefficient * state1 * state2
    return 2.0 * math.sqrt(power) / len(data)


def crc16_ccitt(data: bytes) -> int:
    crc = 0xFFFF
    for value in data:
        crc ^= value << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) & 0xFFFF if crc & 0x8000 else (crc << 1) & 0xFFFF
    return crc


def main() -> None:
    channel_a = make_signal(AMPLITUDE_A, 0.0)
    channel_b = make_signal(AMPLITUDE_B, PHASE_DEG)
    mean = statistics.fmean(channel_a)
    rms = math.sqrt(statistics.fmean([(sample - mean) ** 2 for sample in channel_a]))
    peak_to_peak = max(channel_a) - min(channel_a)
    frequency = estimate_frequency(channel_a)
    phase = estimate_phase(channel_a, channel_b)
    magnitude = goertzel_magnitude(channel_a)

    checks = {
        "stats": abs(mean - OFFSET) <= 1.0
        and abs(rms - math.sqrt(0.5) * AMPLITUDE_A) <= 2.0
        and abs(peak_to_peak - 2.0 * AMPLITUDE_A) <= 2.0,
        "frequency": abs(frequency - SIGNAL_HZ) <= 0.10,
        "phase": abs(phase - PHASE_DEG) <= 0.50,
        "goertzel": abs(magnitude - AMPLITUDE_A) <= 2.0,
        "calibration": abs(((600.0 * 1.0) - 100.0) - 500.0) <= 0.01,
        "pid": abs((2.0 * (10.0 - 8.0)) - 4.0) <= 0.001,
        "crc": crc16_ccitt(b"123456789") == 0x29B1,
    }

    for name, passed in checks.items():
        print(f"REFERENCE,{name.upper()},{'PASS' if passed else 'FAIL'}")
    print(
        "REFERENCE,VALUES,"
        f"MEAN={mean:.3f},RMS={rms:.3f},VPP={peak_to_peak},"
        f"FREQ={frequency:.4f},PHASE={phase:.3f},GOERTZEL={magnitude:.3f}"
    )
    if not all(checks.values()):
        raise SystemExit(1)


if __name__ == "__main__":
    main()
