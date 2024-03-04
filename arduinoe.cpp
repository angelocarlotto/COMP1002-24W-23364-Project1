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

#define NOTE_GS5 831
#define NOTE_FS7 2960

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

Servo servoBase;
LiquidCrystal_I2C lcd(32, 16, 2);
LiquidCrystal_I2C lcd2(33, 16, 2);

int ultrasonicSensorPin[] = {12, 11, 10, 9};
int ultrasonicSensorEntrancePin[] = {7, 8};
int pinFirstSensorEntrance = 7;
int pinSecondSensorEntrance = 8;
int pinLightSensor = A0;
int pinSmokeSensor = A2;
int pinServoMotor = A1;
int pinRelay = 13;
int pinBuzzer = 3;

byte pins1IO = 0b10101010;
byte pins2IO = 0b11111111;
int enderecoChip0 = 39;
int enderecoChip1 = 38;

int minDistance = 60;
int lightSensorValue = 0;
int smokeSensorValue = 0;
int countCarsEntered = 0;
int countCarsEnteredTotal = 0;
int countSpotsAvailable = 0;
int totalSpots = 4;

void initExpander(byte &memori, int address)
{
    Wire.beginTransmission(address);
    Wire.write(~memori);
    Wire.endTransmission();
}

void changePinValue(byte &memori, int address, int pin, bool value)
{

    bitWrite(memori, pin, value);
    Wire.beginTransmission(address);
    Wire.write(~memori);
    Wire.endTransmission();
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

void checkLighSensor()
{
    lightSensorValue = map(analogRead(pinLightSensor), 6, 679, 0, 100);
    if (lightSensorValue < 80)
    {
        digitalWrite(pinRelay, HIGH);
    }
    else
    {
        digitalWrite(pinRelay, LOW);
    }
}

void checkSmokeSensor()
{

    smokeSensorValue = map(analogRead(pinSmokeSensor), 85, 296, 0, 100);

    Serial.println(smokeSensorValue);

    if (smokeSensorValue > 60)
    {

        changePinValue(pins2IO, enderecoChip1, 5, true);
        changePinValue(pins2IO, enderecoChip1, 6, false);
        changePinValue(pins2IO, enderecoChip1, 7, false);

        tone(pinBuzzer, NOTE_FS7, 100);
        delay(500);
        noTone(pinBuzzer);
    }
    else if (smokeSensorValue > 10)
    {
        changePinValue(pins2IO, enderecoChip1, 5, false);
        changePinValue(pins2IO, enderecoChip1, 6, true);
        changePinValue(pins2IO, enderecoChip1, 7, false);

        tone(pinBuzzer, NOTE_GS5, 100);
        delay(1000);
        noTone(pinBuzzer);
    }
    else
    {

        changePinValue(pins2IO, enderecoChip1, 5, false);
        changePinValue(pins2IO, enderecoChip1, 6, false);
        changePinValue(pins2IO, enderecoChip1, 7, true);
    }
}

int gateState = -1;

void openGate()
{

    changePinValue(pins2IO, enderecoChip1, 0, true);
    changePinValue(pins2IO, enderecoChip1, 1, false);
    changePinValue(pins2IO, enderecoChip1, 2, false);
    servoBase.write(90);
    delay(300);
}

void midleStateGate()
{
    changePinValue(pins2IO, enderecoChip1, 0, false);
    changePinValue(pins2IO, enderecoChip1, 1, true);
    changePinValue(pins2IO, enderecoChip1, 2, false);
}
void closeGate()
{

    changePinValue(pins2IO, enderecoChip1, 0, false);
    changePinValue(pins2IO, enderecoChip1, 1, false);
    changePinValue(pins2IO, enderecoChip1, 2, true);
    servoBase.write(0);
    delay(300);
}

void checkEntranceSensors()
{

    float distanceFirst = getDistanceUltrasonic(pinFirstSensorEntrance);
    float distanceSecond = getDistanceUltrasonic(pinSecondSensorEntrance);

    if (distanceFirst > minDistance && distanceSecond > minDistance)
    {
        // gateState=0;
    }

    // first sensor is activated, second is deactivated;
    else if (distanceFirst < minDistance && distanceSecond > minDistance)
    {
        openGate();
        gateState = 1;
    }

    // first  sensor is deactivated and second activated
    else if (gateState == 1 && distanceFirst > minDistance && distanceSecond < minDistance)
    {
        midleStateGate();
        gateState = 2;
    }

    // first  sensor is deactivated and second deactivated
    else if (gateState == 2 && distanceFirst > minDistance && distanceSecond > minDistance)
    {
        closeGate();
        gateState = -1;
    }
}

void checkSpotsSensors()
{
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

            changePinValue(pins1IO, enderecoChip0, 0, distance > minDistance);
            changePinValue(pins1IO, enderecoChip0, 1, distance < minDistance);
            break;
        case 1:
            changePinValue(pins1IO, enderecoChip0, 2, distance > minDistance);
            changePinValue(pins1IO, enderecoChip0, 3, distance < minDistance);
            break;
        case 2:
            changePinValue(pins1IO, enderecoChip0, 4, distance > minDistance);
            changePinValue(pins1IO, enderecoChip0, 5, distance < minDistance);
            break;
        case 3:
            changePinValue(pins1IO, enderecoChip0, 6, distance > minDistance);
            changePinValue(pins1IO, enderecoChip0, 7, distance < minDistance);
            break;
        }
        delay(100);
    }
}

void ajustarTamanhoDaString(String &minhaString)
{
    int tamanhoDesejado = 16;
    // Verificar o comprimento atual da string
    int comprimentoAtual = minhaString.length();

    // Se a string for menor que o tamanho desejado, preencher com espaços em branco
    while (comprimentoAtual < tamanhoDesejado)
    {
        minhaString += " ";
        comprimentoAtual++;
    }

    // Se a string for maior que o tamanho desejado, cortar o excesso
    if (comprimentoAtual > tamanhoDesejado)
    {
        minhaString.remove(tamanhoDesejado);
    }
}

void updateFreeSpotsOnLiquidCristal()
{

    lcd.setCursor(0, 1);
    if (countSpotsAvailable <= 0)
    {
        lcd.print("Parking Packed!");
    }
    else
    {
        lcd.print("Free Spots: " + String(countSpotsAvailable) + "/" + String(totalSpots));
    }

    lcd2.setCursor(0, 0);
    lcd2.print("                ");
    if (gateState == -1)
    {
        lcd2.print(ajustarTamanhoDaString("Entrance free"));
    }
    else if (gateState == 0)
    {
        lcd2.print("On entrance");
    }
    else if (gateState == 1)
    {
        lcd2.print("Entering");
    }
    else if (gateState == 2)
    {
        lcd2.print("Entered");
    }
    else
    {
        lcd2.print("Entrance free");
    }
}

void setup()
{
    Wire.begin();
    Serial.begin(9600);

    servoBase.attach(pinServoMotor);

    pinMode(pinRelay, OUTPUT);
    pinMode(pinLightSensor, INPUT);
    pinMode(pinSmokeSensor, INPUT);
    pinMode(pinServoMotor, OUTPUT);
    pinMode(pinBuzzer, OUTPUT);

    lcd.init();
    lcd2.init();

    lcd.backlight();
    lcd2.backlight();

    lcd.setCursor(0, 0);
    lcd.print("Welcome =]!!");

    lcd2.setCursor(0, 0);
    lcd2.print("Car Parking");

    lcd2.setCursor(0, 1);
    lcd2.print("Auto. Syst. V1.0");

    closeGate();
    // initExpander(pins1IO, enderecoChip0);
    // initExpander(pins2IO, enderecoChip1);
}

void loop()
{

    // checkSmokeSensor();

    //  checkLighSensor();

    checkSpotsSensors();

    checkEntranceSensors();

    updateFreeSpotsOnLiquidCristal();
}
