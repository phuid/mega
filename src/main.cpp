// TODO
// chenge the time perception, so that beep timitngs are calculated from "gamelenght" and not the other way around
//-> gamelenght instead of ticktime

#include <Arduino.h>

#define BEEPLENGTH 100                         // length of every beep
#define WRONG_BEEPSEQ 50, 250, 550, 550, 550   // beep pattern played when the CORRECT wire is disconnected
#define CORRECT_BEEPSEQ 50, 250, 300, 350, 400 // beep pattern played when the WRONG wire is disconnected
#define EXPLOSION_LENGTH 2000                  // length of the last beep & blink
#define GAMELENGTH 40000                       // milliseconds

#define MAX_PIN_NUMBER 43
#define MIN_PIN_NUMBER 24

#define LED LED_BUILTIN
#define SIREN 50
#define RESTART_PIN 53

struct Pin
{
  uint8_t first;
  uint8_t second;
};

Pin correct;

bool previous[10][10];

unsigned long ledtime = 0;
unsigned long ledstart = 0;
bool ledvalue = 0;
unsigned long sirentime = 0;
unsigned long sirenstart = 0;
bool sirenvalue = 0;

unsigned long sequence[5];
unsigned long sequence_start;
bool sequenceactive = 0;

unsigned long gamestart;
uint8_t gameover = 0;

void setup()
{
  Serial.begin(115200);
  Serial.println("Hello World!");

  pinMode(LED, OUTPUT);
  pinMode(SIREN, OUTPUT);
  pinMode(RESTART_PIN, INPUT_PULLUP);

  for (size_t i = MIN_PIN_NUMBER; i <= MAX_PIN_NUMBER; i++)
  {
    pinMode(i, i % 2 == 0 ? INPUT_PULLUP : OUTPUT);
  }

  bool initial[10][10];

  for (size_t i = MIN_PIN_NUMBER; i <= MAX_PIN_NUMBER; i++)
  {
    if (i % 2 == 0)
      continue;

    for (size_t u = MIN_PIN_NUMBER; u <= MAX_PIN_NUMBER; u++)
    {
      if (u % 2 == 1)
        digitalWrite(u, 1);
    }
    digitalWrite(i, 0);
    delay(10);

    for (size_t u = MIN_PIN_NUMBER; u <= MAX_PIN_NUMBER; u++)
    {
      if (u % 2 == 0)
      {
        initial[(i - MIN_PIN_NUMBER) / 2][(u - MIN_PIN_NUMBER) / 2] = digitalRead(u);
      }
    }
  }

  // Serial.print("initial: ");
  // Serial.println();
  // for (size_t i = 0; i < 10; i++)
  // {
  //   for (size_t u = 0; u < 10; u++)
  //   {
  //     Serial.print(initial[i][u]);
  //   }
  //   Serial.println();
  // }

  bool pin_selected = 0;

  do
  {
    bool current[10][10];

    for (size_t i = MIN_PIN_NUMBER; i <= MAX_PIN_NUMBER; i++)
    {
      if (i % 2 == 0)
        continue;

      for (size_t u = MIN_PIN_NUMBER; u <= MAX_PIN_NUMBER; u++)
      {
        if (u % 2 == 1)
          digitalWrite(u, 1);
      }
      digitalWrite(i, 0);
      delay(10);

      for (size_t u = MIN_PIN_NUMBER; u <= MAX_PIN_NUMBER; u++)
      {
        if (u % 2 == 0)
        {
          current[(i - MIN_PIN_NUMBER) / 2][(u - MIN_PIN_NUMBER) / 2] = digitalRead(u);
        }
      }
    }

    for (size_t i = 0; i < 10; i++)
    {
      for (size_t u = 0; u < 10; u++)
      {
        if (current[i][u] != initial[i][u] && !current[i][u])
        {
          correct.first = i * 2 + MIN_PIN_NUMBER;
          correct.second = u * 2 + MIN_PIN_NUMBER;
          pin_selected = 1;

          for (size_t i = 0; i < 10; i++)
          {
            for (size_t u = 0; u < 10; u++)
            {
              previous[i][u] = current[i][u];
            }
          }

          break;
        }
      }
    }

  } while (!pin_selected);

  Serial.println("correct: ");
  Serial.println(correct.first);
  Serial.println(correct.second);
  Serial.println();

  ledtime = 20;
  ledstart = millis();
  ledvalue = 1;
  gamestart = millis();
  gameover = 0;
}

void restart()
{
  Serial.println("restart");
  ledtime = 0;
  sirentime = 0;
  ledstart = 0;
  sirenstart = 0;
  ledvalue = 0;
  sirenvalue = 0;
  gameover = 0;
  for (size_t i = 0; i < sizeof(sequence) / sizeof(sequence[0]); i++)
  {
    sequence[i] = 0;
  }

  gamestart = millis();
}

void led(unsigned long time, bool value)
{
  ledstart = millis();
  ledtime = time;
  ledvalue = value;
}

void led(unsigned long time)
{
  ledstart = millis();
  ledtime = time;
  ledvalue = !ledvalue;
}

void siren(unsigned long time, bool value)
{
  sirenstart = millis();
  sirentime = time;
  sirenvalue = value;
}

void siren(unsigned long time)
{
  sirenstart = millis();
  sirentime = time;
  sirenvalue = !sirenvalue;
}

void makesequence(unsigned long one, unsigned long two, unsigned long three, unsigned long four, unsigned long five)
{
  if (sequenceactive)
    return;
  sequence[0] = one;   // first beep ends
  sequence[1] = two;   // second beep starts
  sequence[2] = three; // second beep ends
  sequence[3] = four;  // third beep starts
  sequence[4] = five;  // third beep ends
  sequenceactive = 1;
  sequence_start = millis();
}

void loop()
{
  if (digitalRead(RESTART_PIN) == 0)
  {
    restart();
  }

  if (!gameover)
  {
    if (ledtime < millis() - ledstart)
    {

// from https://github.com/WouterGritter/CSGO-Bomb/blob/master/armed_state.ino
/**
 * Time between beeps is an exponential function
 *
 * A0 = 1.04865151217746
 * A1 = 0.244017811416199
 * A2 = 1.76379778668885
 *
 * Y(bps) = A0 * E^(A1*X + A2*X^2)
 * X = % OF TIME PASSED (0.0 - 1.0)
 * Y = BEEPS PER SECOND
 */
#define BEEP_A0 1.04865151217746
#define BEEP_A1 0.244017811416199
#define BEEP_A2 1.76379778668885
      // Calculate next beep time
      float x = (float)(millis() - gamestart) / GAMELENGTH;
      float n = BEEP_A1 * x +  BEEP_A2 * x * x;
      float bps = BEEP_A0 * exp(n);

      // Convert bps (beeps per second) to a wait time in milliseconds
      int waitMS = (int)(1000.0 / bps);

      led(waitMS);
      if (!sirenvalue && !sequenceactive)
      {
        siren(BEEPLENGTH, 1);
      }
    }

    if (!sequenceactive && sirentime < millis() - sirenstart)
    {
      sirenvalue = 0;
    }

    bool current[10][10];

    for (size_t i = MIN_PIN_NUMBER; i <= MAX_PIN_NUMBER; i++)
    {
      if (i % 2 == 0)
        continue;

      for (size_t u = MIN_PIN_NUMBER; u <= MAX_PIN_NUMBER; u++)
      {
        if (u % 2 == 1)
          digitalWrite(u, 1);
      }
      digitalWrite(i, 0);

      for (size_t u = MIN_PIN_NUMBER; u <= MAX_PIN_NUMBER; u++)
      {
        if (u % 2 == 0)
        {
          current[(i - MIN_PIN_NUMBER) / 2][(u - MIN_PIN_NUMBER) / 2] = digitalRead(u);
        }
      }
    }

    for (size_t i = 0; i < 10; i++)
    {
      for (size_t u = 0; u < 10; u++)
      {
        if (current[i][u] != previous[i][u] && current[i][u])
        {
          if (i * 2 + MIN_PIN_NUMBER == correct.first && u * 2 + MIN_PIN_NUMBER == correct.second)
          {
            Serial.print((float)(millis() - gamestart) / 1000);
            Serial.println(": correct!");
            makesequence(CORRECT_BEEPSEQ);
            gameover = 1;
          }
          else
          {
            Serial.print((float)(millis() - gamestart) / 1000);
            Serial.println(": wrong!");
            makesequence(WRONG_BEEPSEQ);
          }
        }
        previous[i][u] = current[i][u];
      }
    }
  }
  if (millis() - gamestart > GAMELENGTH && !gameover)
  {
    Serial.print((float)(millis() - gamestart) / 1000);
    Serial.println(": BOOM!");
    gameover = 2;
    led(EXPLOSION_LENGTH, 1);
    siren(EXPLOSION_LENGTH, 1);
  }

  if (sirentime < millis() - sirenstart)
  {
    sirenvalue = 0;
  }

  if (gameover == 1) {
    ledvalue = 0;
    sirenvalue = 0;
  }

  if (GAMELENGTH + EXPLOSION_LENGTH < millis() - gamestart)
  {
    ledvalue = 0;
    sirenvalue = 0;
    sequenceactive = 0;
  }

  if (sequenceactive)
  {
    bool on = 0;
    for (size_t i = 0; i < sizeof(sequence) / sizeof(sequence[0]); i += 2)
    {
      // Serial.print(sequence[i]);
      // Serial.print(" ");
      // Serial.println((millis() - sequence_start < sequence[i - 1] && millis() - sequence_start < sequence[i]) ? 0 : 1);
      if (millis() - sequence_start < sequence[0] || (millis() - sequence_start > sequence[i - 1] && millis() - sequence_start < sequence[i]))
      {
        on = 1;
        break;
      }
    }
    // Serial.print(millis() - sequence_start);
    // Serial.print(" ");
    // Serial.println(on);

    sirenvalue = on;

    sequenceactive = sequence[4] > millis() - sequence_start;
  }

  digitalWrite(LED, ledvalue);
  digitalWrite(SIREN, sirenvalue);
}
