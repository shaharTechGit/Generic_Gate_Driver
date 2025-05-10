/*
  ========================================================================
  Complementary PWM with Hardware Dead Time (Arduino Uno R3 - ATmega328P??????)
  ========================================================================

  - D9  (OC1A): High-side gate signal (non-inverted)
  - D10 (OC1B): Low-side gate signal  (inverted)

  Features:
  - Frequency: 1 kHz (configurable via ICR1)
  - Duty Cycle: ~48.75%
  - Dead Time: ~250 ns between D9 falling and D10 rising
  - Hardware-only: No loop(), no interrupts beyond Timer1
  - Safe for gate drivers like Infineon Eval-1ED3321MC12N
  - Clean timing: Fully controlled via Timer1 compare units

  Limitations:
  - Dead time is only enforced from D9 → D10
  - No dead time from D10 → D9 due to ATmega328P hardware limits

  Author: 
  Last Modified: 	May 4, 2025
  Last modified by: Shahar
*/

const uint16_t ICR1_value   = 16000;   // TOP: sets period (1 kHz at 16 MHz)
// const uint16_t dead_time    = 4;       // Dead time in timer counts (4 * 62.5 ns = 250 ns)
const uint16_t dead_time    = 8;
const uint16_t pulse_width  = 7800;    // D9 ON time (~48.75% duty)

void setup() {
  // Set output pins
  pinMode(9, OUTPUT);   // OC1A (D9)
  pinMode(10, OUTPUT);  // OC1B (D10)

  cli();  // Disable interrupts during timer config

  // Clear Timer1 configuration
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  // Fast PWM, Mode 14: TOP = ICR1
  TCCR1A |= (1 << COM1A1);                   // Non-inverted on D9
  TCCR1A |= (1 << COM1B1) | (1 << COM1B0);   // Inverted on D10
  TCCR1A |= (1 << WGM11);
  TCCR1B |= (1 << WGM12) | (1 << WGM13);
  TCCR1B |= (1 << CS10);  // No prescaler (16 MHz clock)

  // Set frequency and duty
  ICR1  = ICR1_value;                 // PWM period
  OCR1A = pulse_width;               // D9 HIGH duration
  OCR1B = pulse_width + dead_time;   // D10 turns ON after D9 + dead time

  sei();  // Re-enable global interrupts
}

void loop() {
  // Nothing required — everything handled by hardware PWM
}