const uint16_t ICR1_value = 16000;   // 1 kHz PWM frequency
const uint16_t dead_time = 4;        // ~250 ns
const uint16_t pulse_width = 7800;   // ~48.75% duty cycle

const int INPH = 9;   // High-side (D9)
const int INPL = 10;  // Low-side (D10)

volatile uint16_t next_d10_off = 0;
volatile bool d10_on = false;

void setup() {
  pinMode(INPH, OUTPUT);
  pinMode(INPL, OUTPUT);
  digitalWrite(INPH, LOW);
  digitalWrite(INPL, LOW);

  cli();  // Disable interrupts

  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;

  // Fast PWM mode using ICR1 as TOP (Mode 14)
  TCCR1A |= (1 << WGM11);
  TCCR1B |= (1 << WGM13) | (1 << WGM12);

  // No prescaler (16 MHz clock)
  TCCR1B |= (1 << CS10);

  ICR1 = ICR1_value;

  // Enable interrupts
  TIMSK1 |= (1 << TOIE1);   // Overflow (start of cycle)
  TIMSK1 |= (1 << OCIE1A);  // D9 OFF
  TIMSK1 |= (1 << OCIE1B);  // D10 ON

  sei();  // Enable global interrupts
}

ISR(TIMER1_OVF_vect) {
  // Start of cycle
  digitalWrite(INPH, HIGH);   // D9 ON
  digitalWrite(INPL, LOW);    // D10 OFF

  uint16_t start = TCNT1;
  OCR1A = start + pulse_width;                 // D9 OFF
  OCR1B = start + pulse_width + dead_time;     // D10 ON
  next_d10_off = OCR1B + pulse_width;          // Schedule D10 OFF
  d10_on = true;
}

ISR(TIMER1_COMPA_vect) {
  digitalWrite(INPH, LOW);  // D9 OFF
}

ISR(TIMER1_COMPB_vect) {
  digitalWrite(INPL, HIGH); // D10 ON
}

// D10 OFF logic in loop — safe because Fast PWM is monotonic
void loop() {
  if (d10_on && TCNT1 >= next_d10_off) {
    digitalWrite(INPL, LOW);
    d10_on = false;
  }
}