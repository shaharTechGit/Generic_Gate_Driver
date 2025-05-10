/*
  ============================================================================
  Manual Complementary PWM with Dead Time - Arduino Uno
  ============================================================================
  - Frequency: 1 kHz (adjustable)
  - Dead time: ~1 µs (adjustable but 1us minimum)
  - Outputs:
      - D9 (INPH) → High-side gate signal
      - D10 (INPL) → Low-side gate signal
	  - D13 (ResetN)
  - Purpose:
      - Generate complementary PWM signals
      - Insert safe dead time to prevent shoot-through
      - Ensure clean switching for gate drivers (e.g., Eval-1ED3321MC12N)

  - Author: []
  - Last Updated: [5 May 2025]
  - Last Updated by: Shahar
  ============================================================================
*/

// =============================
// USER-CONFIGURABLE PARAMETERS
// =============================

// PWM settings
const int pwm_freq = 1000;                // Desired PWM frequency in Hz (e.g., 1 kHz)
const int duty_us = 498;                  // ON time (pulse width) in microseconds (~49.8% duty cycle)
const int dead_time_us = 1;                // Dead time between D9 falling and D10 rising [µs]

// =============================
// PIN DEFINITIONS
// =============================

const int resetN = 13; // Reset control pin (D13)
const int INPH = 9;   // High-side PWM output pin (D9)
const int INPL = 10;  // Low-side PWM output pin (D10)

// =============================
// DERIVED PARAMETERS
// =============================

const int period_us = 1000000 / pwm_freq; // Total period of PWM signal in microseconds (e.g., 1000 µs for 1 kHz)

// =============================
// SETUP FUNCTION
// =============================

void setup() {
  // Set output pins
  pinMode(INPH, OUTPUT);
  pinMode(INPL, OUTPUT);
  pinMode(resetN, OUTPUT);

  // Initialize outputs LOW
  digitalWrite(INPH, LOW);
  digitalWrite(INPL, LOW);
  digitalWrite(resetN, HIGH);        // <--- Keep board enabled by default
}

void resetEvalBoard() {
  digitalWrite(resetN, LOW);  // Force board reset
  delay(10);                  // Wait 10 ms
  digitalWrite(resetN, HIGH); // Release reset
}

// =============================
// MAIN LOOP
// =============================

void loop() {
  static unsigned long last_cycle = 0;    // Time of last PWM cycle start
  unsigned long now = micros();            // Current time in microseconds

  // Start a new PWM cycle every 'period_us' microseconds
  if (now - last_cycle >= period_us) {
    last_cycle = now;  // Update cycle timestamp

    // -------------------------------------------------------
    // 1. Turn ON High-Side (D9)
    // -------------------------------------------------------
    digitalWrite(INPH, HIGH);              // D9 High-side ON
    delayMicroseconds(duty_us);            // Keep ON for desired duty cycle
    digitalWrite(INPH, LOW);               // D9 High-side OFF

    // -------------------------------------------------------
    // 2. Insert Dead Time
    // -------------------------------------------------------
    delayMicroseconds(dead_time_us);       // Short dead time before turning on low-side

    // -------------------------------------------------------
    // 3. Turn ON Low-Side (D10)
    // -------------------------------------------------------
    digitalWrite(INPL, HIGH);              // D10 Low-side ON
    delayMicroseconds(duty_us);            // Keep ON for desired duty cycle
    digitalWrite(INPL, LOW);               // D10 Low-side OFF

    // -------------------------------------------------------
    // 4. Final Dead Time (before next cycle)
    // -------------------------------------------------------
    delayMicroseconds(dead_time_us);       // Short dead time before next high-side ON
  }
}