/*
  Four digit seven-segment display demonstration

  Program that displays a floating-point counter on a four-digit seven-segment
  display. It is associated with the Four Digit Seven Segment Display blog post
  at https://lagacemichel.com

  MIT License

  Copyright (c) 2021, Michel Lagace

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

// Digital output pins to turn on or off each segment of the selected digit
#define SEGMENT_A 2
#define SEGMENT_B 3
#define SEGMENT_C 4
#define SEGMENT_D 5
#define SEGMENT_E 6
#define SEGMENT_F 7
#define SEGMENT_G 8
#define SEGMENT_DP 9

// Digital output pins to select each of the four digits
#define DIGIT_1 10
#define DIGIT_2 11
#define DIGIT_3 12
#define DIGIT_4 13
const int numberOfDigits = 4;

// Number of microseconds to leave digit on to achieve 200 Hz
const int digitTimeOn = 1250;

// Interval in seconds between counter increments - 0.2 sec for 5 counts/sec
const float timeBetweenIncrements = 0.2;
const float microsecondsInASecond = 1000000.0;

// Number of digits after decimal points and floating-point constants
const int scaleOfNumber = 1; // number of digits after decimal point
const int scalingFactor = pow(10.0, scaleOfNumber);
const float increment = 1.0 / scalingFactor;
const float minimumValue = -pow(10.0, numberOfDigits - 1) / scalingFactor + increment;
const float maximumValue = pow(10.0, numberOfDigits) / scalingFactor - increment;

// Counters to control value displayed on seven-segment display
float displayCounter = minimumValue;
long iterations = 0;

// Segment encodings for digits 0, 1, 2, 3, 4, 5, 6, 7, 8, and 9 and for the
// minus // sign '-'. Each bit represents a segment in the following order:
// n/a, a, b, c, d, e, f, g. The most significant bit is not used. Segments
// are identified as follows
//          a
//      +-------+
//      |       |
//     f|       |b
//      |   g   |
//      +-------+
//      |       |
//     e|       |c
//      |       |
//      +-------+  . dp
//          d
const int segmentPatterns[] = {
  0b1111110, // 0: a, b, c, d, e, f
  0b0110000, // 1: b, c
  0b1101101, // 2: a, b, d, e, g
  0b1111001, // 3: a, b, c, d, g
  0b0110011, // 4: b, c, f, g
  0b1011011, // 5: a, c, d, f, g
  0b1011111, // 6: a, c, d, e, f, g
  0b1110000, // 7: a, b, c
  0b1111111, // 8: a, b, c, d, e, f, g
  0b1111011, // 9: a, b, c, d, f, g
};

// void extinguishDigits()
// Extinguish all digits in the four digit seven-segment display
void extinguishDigits()
{
  // Float all common cathodes by setting all digit pins as INPUT
  for (int currentDigit = DIGIT_1; currentDigit <= DIGIT_4; currentDigit++)
  {
    pinMode(currentDigit, INPUT);
  }
}

// void displayMinusSign(digit)
// Displays a minus sign on the specified digit.
void displayMinusSign(int digit)
{
  // Extinguish all digits
  extinguishDigits();

  // Turn off all digit segments
  for (int currentSegment = SEGMENT_F; currentSegment >= SEGMENT_A; currentSegment--)
  {
    digitalWrite(currentSegment, LOW);
  }

  // Turn off decimal point segment
  digitalWrite(SEGMENT_DP, LOW);

  // Turn on g segment to display minus sign ('-')
  digitalWrite(SEGMENT_G, HIGH);

  // Activate requested digit
  int selectedDigitPin = digit + DIGIT_1 - 1;
  pinMode(selectedDigitPin, OUTPUT);
  digitalWrite(selectedDigitPin, LOW);
}

// void displayDigit(digit, value, decimalPoint)
// Display a value, between 0 and 9 on the specified digit of the display
// and display the decimal point if required.
void displayDigit(int digit, int value, bool decimalPoint)
{
  // Extinguish all digits
  extinguishDigits();

  // Prepare the segments to light
  int segments = 0;
  if ((value >= 0) && (value <= 9))
  {
    segments = segmentPatterns[value];
  }

  // turn on or off each segment
  for (int currentSegment = SEGMENT_G; currentSegment >= SEGMENT_A; currentSegment--)
  {
    digitalWrite(currentSegment, segments & 0b00000001);
    segments = segments >> 1;
  }

  // Set the decimal point
  digitalWrite(SEGMENT_DP, decimalPoint);

  // Select requested digit
  int selectedDigitPin = digit + DIGIT_1 - 1;
  pinMode(selectedDigitPin, OUTPUT);
  digitalWrite(selectedDigitPin, LOW);
}

// void displayNumber(number)
// Display the number passed as a parameter on the four digit
// seven-segment display.
void displayNumber(float number)
{
  // Process digits from right to left
  int currentDigit = numberOfDigits;

  // Only numbers smaller than the maximum value can be displayed
  if ((number >= minimumValue) && (number <= maximumValue))
  {
    // Scale the number and extract decimal and integer portions of number
    bool negative = false;
    if (number < 0.0)
    {
      negative = true;
      number = -number;
    }
    int integerPart = number;
    int fractionalPart = (number - integerPart) * scalingFactor;

    // Display fractional part
    int remainingValue = fractionalPart;
    for (int i = 0; i < scaleOfNumber; i++)
    {
      int digit = remainingValue % 10;
      remainingValue = remainingValue / 10;
      displayDigit(currentDigit--, digit, false);
      delayMicroseconds(digitTimeOn);
    }

    // Display integer part
    remainingValue = integerPart;
    bool decimalPoint = true;
    do
    {
      int digit = remainingValue % 10;
      remainingValue = remainingValue / 10;
      displayDigit(currentDigit--, digit, decimalPoint);
      decimalPoint = false;
      delayMicroseconds(digitTimeOn);
    } while (remainingValue > 0);

    // Display minus sign if number is negative
    if (negative)
    {
      displayMinusSign(currentDigit--);
      delayMicroseconds(digitTimeOn);
    }
  }

  // Extinguish all digits ensuring that all digits are displayed the same amount of time
  // and wait a bit if not all digits were lit, ensuring constant intensity for all numbers
  // currentDigit actually contains the number of digits left.
  extinguishDigits();
  delayMicroseconds(digitTimeOn * currentDigit);
}

// void setup()
// Set all digital pins used for digit selection as input to extinguish all digits
// and set display segments as outputs.
void setup() {

  // Float all digit selection digital pins as input
  extinguishDigits();

  // Set all segment digital pins as output and turn them off
  for (int currentSegment = SEGMENT_G; currentSegment >= SEGMENT_A; currentSegment--)
  {
    pinMode(currentSegment, OUTPUT);
  }

  // Set decimal point digital pin as output and turn it off
  pinMode(SEGMENT_DP, OUTPUT);

  // Reset counters
  displayCounter = minimumValue;
  iterations = 0;
}

// void loop()
// Continuously display the counter and increment it at the specified interval.
void loop() {
  // Increment number of iterations until the interval between increments has been reached
  // Each iteration takes digitTimeOn*numberOfDigits microseconds
  iterations++;
  if ((iterations * digitTimeOn * numberOfDigits / microsecondsInASecond) > timeBetweenIncrements)
  {
    // Reset loop counter and increment display counter
    iterations = 0;
    displayCounter += increment;

    // Reset display counter if it has reached the maximum value
    if (displayCounter > maximumValue)
    {
      displayCounter = minimumValue;
    }
  }

  // Display floating-point counter value
  displayNumber(displayCounter);
}
