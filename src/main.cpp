#include <Arduino.h>

#define MAX_PIN_NUMBER 43
#define MIN_PIN_NUMBER 24

struct Pin
{
  uint8_t first;
  uint8_t second;
};

Pin correct;

bool previous[10][10];

void setup()
{
  Serial.begin(115200);
  Serial.println("Hello World!");
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

  Serial.print("initial: ");
  Serial.println();
  for (size_t i = 0; i < 10; i++)
  {
    for (size_t u = 0; u < 10; u++)
    {
      Serial.print(initial[i][u]);
    }
    Serial.println();
  }

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
}

void loop()
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
      if (current[i][u] != previous[i][u] && current[i][u])
      {
        Serial.println(i * 2 + MIN_PIN_NUMBER == correct.first && u * 2 + MIN_PIN_NUMBER == correct.second ? "correct" : "wrong");
      }
    }
  }
}
