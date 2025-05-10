/* 
   ====================================================================
   Arduino Uno - Complementary PWM Signal Generation (1 kHz, 2 kHz, 16kHz) for 
   Infineon Eval-1ED3321MC12N Gate Driver
   With IGBT Transistors Infineon-IKW40N120H3
   ====================================================================
   This code generates **complementary PWM signals** 
   on pins **D9 (INPH)** and **D10 (INPL)** using Timer1 on the Arduino Uno. 
   The **Infineon Eval-1ED3321MC12N** expects proper complementary signals 
   with a dead time to avoid shoot-through (both switches ON simultaneously). 
   The PWM signals are **hardware-controlled**, ensuring precision and stability.

   ====================================================================
   | How It Works                                                     |
   ====================================================================
   1. **Timer1 Configuration**:
      - Timer1 is set to **Fast PWM mode (Mode 14)**.
      - The **ICR1 register** determines the PWM frequency.
      - The **OCR1A and OCR1B registers** set the duty cycle.
      - **D9 (INPH) is non-inverted**, while **D10 (INPL) is inverted**.

   2. **Complementary PWM with Dead Time**:
      - **D9 (INPH) is HIGH while D10 (INPL) is LOW**.
      - A **small dead time** ensures that both are LOW before switching.

   3. **Reset Pin (RSTN) Handling**:
      - The Eval board **requires RESET (RSTN) HIGH** to enable operation.
      - The **RESET pin (D13) is set HIGH** by default.
      - A function `resetEvalBoard()` allows manual reset if needed.

   4. **PWM Frequency Calculation**:
      The formula for calculating the PWM frequency is:
         f_PWM = (16 MHz) / (2 * ICR1)
      For **ICR1 = 8000**, the resulting frequency is:
         f_PWM = (16,000,000) / (2 * 8000) = **1 kHz**
   
   ====================================================================
   | How to Adjust PWM Settings                                        |
   ====================================================================
   - **Changing the PWM Frequency**:
     Modify `ICR1_value` in the code:
       * `ICR1 = 4000;` → **2 kHz**
       * `ICR1 = 8000;` → **1 kHz** (default)
       * `ICR1 = 16000;` → **500 Hz**

   - **Adjusting the Duty Cycle**:
     Modify `OCR1A_value` and `OCR1B_value`:
       * `OCR1A = ICR1 * 0.50;` → **50% duty cycle (default)**
       * `OCR1A = ICR1 * 0.75;` → **75% duty cycle**
       * `OCR1A = ICR1 * 0.25;` → **25% duty cycle**
     Ensure `OCR1B` is adjusted similarly, accounting for **dead time**.

   - **Modifying the Dead Time**:
     Increase or decrease `dead_time`:
       * `dead_time = 200;` → **More dead time (~25 µs at 1 kHz)**
       * `dead_time = 50;` → **Less dead time (~6.25 µs at 1 kHz)**
     The values for `OCR1A` and `OCR1B` automatically update.

   ====================================================================
   | Hardware Setup                                                    |
   ====================================================================
   - **PWM Outputs**:
       * Pin **D9 (INPH)** → High-side gate signal
       * Pin **D10 (INPL)** → Low-side gate signal

   - **Interrupt Inputs** (to stop PWM when triggered):
       * Pin **D2** → Stops PWM when triggered
       * Pin **D3** → Stops PWM when triggered

   - **Reset Pin (RSTN)**:
       * Pin **D13** → Connected to the Eval board RSTN pin
       * **Must be HIGH** for normal operation
       * Can be manually toggled using `resetEvalBoard()`

   - **Serial Communication**:
       * Prints debug messages via Serial Monitor (9600 baud)

   ====================================================================
   | Author: []
   | Last Update: [29 January 2025]
   | Last Eddited By: [Shahar]
   ====================================================================
*/


// Interrupt pins (used to stop PWM when triggered)
const byte intr1 = 2;  
const byte intr2 = 3;  

// PWM output pins (Timer1 controls these)
const byte INPH = 9;  
const byte INPL = 10;  
const byte resetN = 13; 

// our timer clock is 16 MHz, 1 timer count = 62.5 ns.
// So to get ~6250ns = 6.25us dead-time: dead_time = 100 --> 100 * 62.5ns = 6250ns = 6.25us

// Timer1 parameters for 1 kHz PWM  ;  ICR1_value = 8000 and dead_time = 100  --> dead time ~ 6.25us
const int ICR1_value = 8000;   
const int dead_time = 100;     
int OCR1A_value = 4000 - (dead_time / 2);
int OCR1B_value = 4000 + (dead_time / 2);

// // Timer1 parameters for 1 kHz PWM, more dead time
// const int ICR1_value = 8000;   
// const int dead_time = 800;     
// int OCR1A_value = 4000 - (dead_time / 2);
// int OCR1B_value = 4000 + (dead_time / 2);

// // Timer1 parameters for 1 kHz PWM , Duty Cycle 50% (1/2)
// const int ICR1_value = 8000;   
// const int dead_time = 100;     
// int OCR1A_value = (ICR1_value / 2) - (dead_time / 2);
// int OCR1B_value = (ICR1_value / 2) + (dead_time / 2);

// // Timer1 parameters for 1 kHz PWM, Duty cycle 25% 1/4
// const int ICR1_value = 8000;   
// const int dead_time = 100;     
// int OCR1A_value = (ICR1_value / 4) - (dead_time / 2);
// int OCR1B_value = (ICR1_value / 4) + (dead_time / 2);

// // Timer1 parameters for 2 kHz PWM , Duty Cycle 50%
// const int ICR1_value = 2000;   
// const int dead_time = 100;     
// int OCR1A_value = (ICR1_value / 2) - (dead_time / 2);
// int OCR1B_value = (ICR1_value / 2) + (dead_time / 2);

// // Timer1 parameters for 16 kHz PWM , Duty Cycle 50%
// const int ICR1_value = 1000;   
// const int dead_time = 100;     
// int OCR1A_value = (ICR1_value / 2) - (dead_time / 2);
// int OCR1B_value = (ICR1_value / 2) + (dead_time / 2);

// // Timer1 parameters for 1/2 kHz PWM , Duty Cycle 50%
// const int ICR1_value = 16000;   
// const int dead_time = 100;     
// int OCR1A_value = (ICR1_value / 2) - (dead_time / 2);
// int OCR1B_value = (ICR1_value / 2) + (dead_time / 2);

void setup() {
  pinMode(intr1, INPUT_PULLUP);
  pinMode(intr2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(intr1), stopPWM, CHANGE);
  attachInterrupt(digitalPinToInterrupt(intr2), stopPWM, CHANGE);
  
  pinMode(INPH, OUTPUT);
  pinMode(INPL, OUTPUT);
  
  pinMode(resetN, OUTPUT);
  digitalWrite(resetN, HIGH);  

  Serial.begin(9600);

  setupPWM();
}

void loop() {
  delay(1000);
}

void setupPWM() {
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;

  DDRB |= (1 << DDB1) | (1 << DDB2); 

  TCCR1A |= (1 << COM1A1);  
  TCCR1A |= (1 << COM1B1) | (1 << COM1B0);  
  TCCR1A |= (1 << WGM11);  
  TCCR1B |= (1 << WGM12) | (1 << WGM13);  
  TCCR1B |= (1 << CS10);  

  ICR1 = ICR1_value; 

  // Ensure duty cycle values are within limits
  if (OCR1A_value < 0) OCR1A_value = 0;
  if (OCR1B_value > ICR1_value) OCR1B_value = ICR1_value;

  OCR1A = OCR1A_value; 
  OCR1B = OCR1B_value; 

  Serial.println("PWM started at 1 kHz with complementary signals and dead time");
}

void stopPWM() {
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  digitalWrite(resetN, LOW);  

  Serial.println("PWM stopped, Eval board reset");
}

void resetEvalBoard() {
  Serial.println("Resetting Eval board...");
  digitalWrite(resetN, LOW);  
  delay(10);                 
  digitalWrite(resetN, HIGH); 
}
