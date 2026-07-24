# Host Tests

These tests exercise pure C modules without STM32 registers: PI anti-windup, command ramping, state transitions, motor mixing, and `uint32_t` timeout wraparound.

From the repository root, use a host GCC compiler:

```powershell
gcc -std=c99 -Wall -Wextra -IConfig -IApp -IControl tests/host/test_control.c App/app_state.c App/remote_control.c Control/pid.c Control/filters.c Control/motor_mixer.c Control/balance_controller.c -lm -o tests/host/test_control.exe
./tests/host/test_control.exe
```

The tests do not validate PCB wiring, IMU orientation, motor polarity, encoder polarity, or control gains. Those require the staged hardware checklist in `docs/06_bringup_checklist.md`.
