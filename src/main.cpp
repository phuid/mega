#include <Arduino.h>

#define MAX_PIN_NUMBER 43
#define MIN_PIN_NUMBER 24

#define LED 50
#define SIREN LED_BUILTIN

struct Pin
{
  uint8_t first;
  uint8_t second;
};

Pin correct;

bool previous[10][10];

#define DIVISION_CONST 40

unsigned long ledtime = 0;
unsigned long ledstart = 0;
bool ledvalue = 0;
unsigned long sirentime = 0;
unsigned long sirenstart = 0;
bool sirenvalue = 0;
unsigned long ticktime = 1000;

bool gameover = 0;

void setup()
{
  Serial.begin(115200);
  Serial.println("Hello World!");

  pinMode(LED, OUTPUT);
  pinMode(SIREN, OUTPUT);

  for (size_t i = MIN_PIN_NUMBER; i <= MAX_PIN_NUMBER; i++)
  {
    pinMode(i, i % 2 == 0 ? INPUT_PULLUP : OUTPUT);
  }
  pinMode(53, OUTPUT);

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

void loop()
{
  if (!gameover)
  {
    if (ledtime < millis() - ledstart)
    {
      led (ticktime);
      if (!sirenvalue)
      {
        siren(20, 1);
      }
      Serial.println(ticktime);

      if (ticktime > 2)
        ticktime -= ticktime / DIVISION_CONST;
    }

    if (sirentime < millis() - sirenstart)
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
      delay(2);

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
            Serial.println("correct!");
            gameover = 1;
          }
          else
          {
            Serial.println("wrong!");
            siren(500, 1);
          }
        }
        previous[i][u] = current[i][u];
      }
    }
  }
  if (ticktime < 70 && !gameover)
  {
    gameover = 1;
    ledvalue = 1;
    sirenstart = millis();
    sirentime = 2000;
    sirenvalue = 1;
  }
  if (gameover)
  {
    if (sirentime < millis() - sirenstart)
    {
      ledvalue = 0;
      sirenvalue = 0;
    }
  }
  digitalWrite(LED, ledvalue);
  digitalWrite(SIREN, sirenvalue);
}
