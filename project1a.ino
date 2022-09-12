//Jon Grote

//definitely need the gamepad module
#define INCLUDE_GAMEPAD_MODULE
#define INCLUDE_TERMINAL_MODULE
#define INCLUDE_PINMONITOR_MODULE
#define CUSTOM_SETTINGS
#define INCLUDE_LEDCONTROL_MODULE

//must have the dabble library to communicate through the application to the blutooth
#include <Dabble.h>
#include <Servo.h>

//pin declarations:

//push button pin
//const int buttonPin = 40;

//Left wheel direction
const int LDIR = 41;
//right wheel direction
const int RDIR = 42;
//left pwm
const int LPWM = 45;
//right pwm
const int RPWM = 46;
// era pin
const int era = 18;
//ela pin
const int ela = 19;

const int gripper = 10;
// Declare servo object
Servo gripper_servo;
Servo lifter;
Servo tilter;


float X = 50.0;
//the dutycycle is now ready for an integer value corresponding to the controler's wishes. start value = 50%
float dutyCycle = 255.00 * (X / 100.00);

//set up variables for 2 debounce delays
bool currentButtonState;
bool currentButtonState2;
const int debounceDelay = 1000;

bool lastSteadyState = false;
bool lastSteadyState2 = false;

bool lastButtonState = false;
bool lastButtonState2 = false;

unsigned long lastDebounceTime = 0;
unsigned long lastDebounceTime2 = 0;

void setup()
{
    //declare the pinmodes for the wheel motors
    pinMode(LDIR, OUTPUT);
    pinMode(RDIR, OUTPUT);
    pinMode(LPWM, OUTPUT);
    pinMode(RPWM, OUTPUT);

    //declare pinmode for gripper motor position reading
    pinMode(A0, INPUT);


    // Attach servo pins to objects
    gripper_servo.attach(gripper);
    tilter.attach(11);
    lifter.attach(3);

    //being Serial communication between the mega and computer and the mega and the Blutooth using Dabble library
    Serial.begin(115200);
    Dabble.begin(9600);

} //###### end of setup routine#######

void loop()
{
    //my tilt motor is broken. everything else works. i set a value here that i estimate to be 
    //roughly parallel to the ground. 
    tilter.writeMicroseconds(1550);
    Dabble.processInput(); // This line is crucial in grabbing our data

    if (GamePad.isCirclePressed()) /* CONTROLS THE GRIPPER opening*/
    {
        //tilter.writeMicroseconds(1500);
        for (int pos = 2400; pos >= 500; pos -= 100)
        {
            float GPos = 5.0 * analogRead(A0) / 1023.00;
            // Serial.println(GPos);
            float calcpos = 1159.1 * GPos - 69.383;
            Serial.print("calcpos = ");
            Serial.print(calcpos);
            Serial.print(" pos =");
            Serial.println(pos);
            gripper_servo.writeMicroseconds(pos);
            delay(150);
        }
    }
    else if (GamePad.isSquarePressed()) //CONTROLS THE GRIPPER CLOSING
    {
        //tilter.writeMicroseconds(1550);
        for (int pos = 500; pos <= 2400; pos += 100)
        {
            gripper_servo.writeMicroseconds(pos);
            delay(250);
            // Serial.print("Gripper Position=");
            float GPos = 5.0 * analogRead(A0) / 1023.00;
            // Serial.println(GPos);

            /*
            i will make sure to include more of why this is here in my notebook,
            but essentially i plotted the analog values returned for each position
            that was written to the servo (or you know, for every 100 pwm increment)
            thus a trendline equation was found that equates the written position (pos)
            to the expected (calculated) position. when the values are too far apart, this 
            means the arm has grasped an object, and triggers the arm to be raised  */
            float calcpos = 1159.1 * GPos - 69.383;
            Serial.print("calcpos = ");
            Serial.print(calcpos);
            Serial.print(" pos =");
            Serial.println(pos);
            if ((pos - calcpos) > 65)
            {
                STOP(); //calls a function that STOPS everything and raises the arm
                break;
            }
        } //close for loop
    }
    else if (GamePad.isTrianglePressed() == true)//a sweep that raises the arm of the robot "smoothly
    {
        tilter.writeMicroseconds(1200);
        for (int pos = 1900; pos >= 1000; pos -= 25)
        {
            lifter.writeMicroseconds(pos);
            delay(100);
        }
    }
    else if (GamePad.isCrossPressed() == true) // lower the arm
    {
        for (int pos = 1000; pos <= 1900; pos += 25)
        {
            lifter.writeMicroseconds(pos);
            delay(100);
        }
    }
    else if (GamePad.isUpPressed() == true) //move forward
    {
        digitalWrite(RDIR, LOW);
        digitalWrite(LDIR, LOW);
        analogWrite(RPWM, dutyCycle);
        analogWrite(LPWM, dutyCycle);
        //Serial.println("all of tinseltown is at DEFCON 5 until their diabolically displaced 'D' is demonstrably displayed once more. can we cool it with the alliteration?");
    }

    //if down is pressed, drive both motors backwards at dutycycle%. LOW is forwards
    else if (GamePad.isDownPressed() == true)
    {
        digitalWrite(RDIR, HIGH);
        digitalWrite(LDIR, HIGH);
        analogWrite(RPWM, dutyCycle);
        analogWrite(LPWM, dutyCycle);
    }

    //if left is pressed, drive the right motor forwards at dutycycle%.
    //drive the left motor backwards at dutycycle%
    else if (GamePad.isLeftPressed() == true)
    {
        digitalWrite(RDIR, LOW);
        digitalWrite(LDIR, HIGH);
        analogWrite(RPWM, dutyCycle);
        analogWrite(LPWM, dutyCycle);
    }

    //if right is pressed, drive the right motor backwards at dutycycle
    //drive the left motor forwards
    else if (GamePad.isRightPressed() == true)
    {
        digitalWrite(RDIR, HIGH);
        digitalWrite(LDIR, LOW);
        analogWrite(RPWM, dutyCycle);
        analogWrite(LPWM, dutyCycle);
    }
    else //if no inputs are given, write both motors to dutycycle of 0%, stopping them
    {
        analogWrite(RPWM, 0);
        analogWrite(LPWM, 0);
    }

    /*
    multiple boolean true signals are sent when the select button is pressed,
    thus trying to use the normal if() statement will be much like using a 
    pushbutton in that it will generate somewhat aberrant behavior. a "tap"
    may send 10 signals, but holding the select button will send dozens. to get around this
    a debounce is needed so that within a specified time frame [1 second], 
    only 1 press will be significant,
    and any other presses in that time frame will be ignored  */
    currentButtonState = GamePad.isSelectPressed();

    if ((millis() - lastDebounceTime) > debounceDelay)
    {
        if (currentButtonState == true && lastButtonState == false)
        {
            dutyCycle = dutyCycle + 25.5;
            lastDebounceTime = millis();
        }
        else if (currentButtonState == true && lastButtonState == true)
        {
            lastDebounceTime = millis();
        }
        lastButtonState = currentButtonState;
    }

    // set up another debounce delay for the start button
    currentButtonState2 = GamePad.isStartPressed();

    if ((millis() - lastDebounceTime2) > debounceDelay)
    {
        if (currentButtonState2 == true && lastButtonState2 == false)
        {
            dutyCycle = dutyCycle - 25.5;
            lastDebounceTime2 = millis();
        }
        else if (currentButtonState2 == true && lastButtonState2 == true)
        {
            lastDebounceTime2 = millis();
        }
        lastButtonState2 = currentButtonState2;
    }

    Serial.print("Gripper Position=");
    Serial.println(5.0 * analogRead(A0) / 1023);
    Serial.println(dutyCycle);

} // ########## end of loop routine #######

void STOP() // 
{
    for (int pos = 1900; pos >= 1000; pos -= 50)
    {
        lifter.writeMicroseconds(pos);
        delay(100);
        if (pos == 1000)
        {
            break;
        }
    }
    //lifter.writeMicroseconds(1000);
}