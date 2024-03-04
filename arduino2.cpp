
/* PCF8574

PinA2 PinA1 PinA0  Address
  0     0     0    0x20  0 32
  0     0     1    0x21  1 33
  0     1     0    0x22  2 34
  0     1     1    0x23  3 35
  1     0     0    0x24  4 36
  1     0     1    0x25  5 37
  1     1     0    0x26  6 38
  1     1     1    0x27  7 39

*/

/*

      R	G	R	G				L
      P7	P6	P5	P4	P3	P2	P1	P0
                                    1
                                1	0
                            1	0	0
      1		0	1	0	0	0	0	0
      0		1	0	1	0	0	0	0

      128	64	32	16	8	4	2	1

*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

Servo servoBase;
LiquidCrystal_I2C lcd(32, 16, 2);
LiquidCrystal_I2C lcd2(33, 16, 2);

bool toogleLights = false;

byte pins1IO = 0b10101010;
int enderecoChip0 = 39;
int enderecoChip1 = 38;

int ultrasonicSensorPin[] = {12, 11, 10, 9};
int ultrasonicSensorEntrancePin[] = {7, 8};

int minDistance = 60;
int pinLightSensor = A0;
int pinRelay = 13;
int light = 0;
int countCarsEntered = 0;
int countCarsEnteredTotal = 0;
int countSpotsAvailable = 0;
int totalSpots = 4;

void initExpander(int address)
{
    Wire.beginTransmission(address);
    Wire.write(~pins1IO);
    Wire.endTransmission();
}

void changePinValue(int address, int pin, bool value)
{
    bitWrite(pins1IO, pin, value);
    Wire.beginTransmission(address);
    Wire.write(~pins1IO);
    Wire.endTransmission();
}

void setup()
{

    servoBase.attach(A1); // Pin a utilizar para servo
    //   servoBase.write(0);  //asigno 0 al servo motor

    Wire.begin();
    Serial.begin(9600);

    lcd.init();  // initialize the lcd
    lcd2.init(); // initialize the lcd

    // Print a message to the LCD.
    lcd.backlight();
    lcd2.backlight();

    lcd.setCursor(0, 0);
    lcd.print("Welcome =]!!");

    lcd2.setCursor(0, 0);
    lcd2.print("Car Parking");

    lcd2.setCursor(0, 1);
    lcd2.print("Auto. Syst. V1.0");

    pinMode(pinRelay, OUTPUT);
    pinMode(2, INPUT);
    pinMode(pinLightSensor, INPUT);

    attachInterrupt(digitalPinToInterrupt(2), toggleLigths, RISING);

    initExpander(enderecoChip0);
}

float getDistanceUltrasonic(int pin)
{
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    delayMicroseconds(2);

    digitalWrite(pin, HIGH);
    delayMicroseconds(5);

    digitalWrite(pin, LOW);
    pinMode(pin, INPUT);
    float duration = pulseIn(pin, HIGH);
    float distance = (duration * 0.034) / 2;

    return distance;
}

void loop()
{

    light = map(analogRead(pinLightSensor), 6, 679, 0, 100);
    Serial.println(light);
    if (light < 80)
    {
        digitalWrite(pinRelay, HIGH);
    }
    else
    {
        digitalWrite(pinRelay, LOW);
    }

    for (int i = 0; i < 2; i++)
    {

        float distance = getDistanceUltrasonic(ultrasonicSensorEntrancePin[i]);

        switch (i)
        {
        case 0:
            if (distance < minDistance)
            {
                servoBase.write(90);
            }
            else
            {
                servoBase.write(0);
            }
            changePinValue(enderecoChip1, 0, distance < minDistance);
            changePinValue(enderecoChip1, 1, distance > minDistance);
            break;
        case 1:

            break;
        }
        delay(100);
    }

    countSpotsAvailable = 0;
    for (int i = 0; i < totalSpots; i++)
    {
        float distance = getDistanceUltrasonic(ultrasonicSensorPin[i]);

        if (distance > minDistance)
        {
            countSpotsAvailable++;
        }

        switch (i)
        {
        case 0:

            changePinValue(enderecoChip0, 0, distance > minDistance);
            changePinValue(enderecoChip0, 1, distance < minDistance);
            break;
        case 1:
            changePinValue(enderecoChip0, 2, distance > minDistance);
            changePinValue(enderecoChip0, 3, distance < minDistance);
            break;
        case 2:
            changePinValue(enderecoChip0, 4, distance > minDistance);
            changePinValue(enderecoChip0, 5, distance < minDistance);
            break;
        case 3:
            changePinValue(enderecoChip0, 6, distance > minDistance);
            changePinValue(enderecoChip0, 7, distance < minDistance);
            break;
        }
        delay(100);
    }

    lcd.setCursor(0, 1);
    lcd.print("Free Spots:" + String(countSpotsAvailable) + "/" + String(totalSpots));
}

void toggleLigths()
{
    toogleLights = !toogleLights;
    digitalWrite(pinRelay, toogleLights);
}
