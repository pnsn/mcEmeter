 /* Measures the long term energy usage of a load by sampling its DC current and
  voltage. Both may vary with time. The base sample rate is about 1750 sps for a sample
  time of about 570 us. Energy consumption is E = V * I * t. Because sampling is not
  continuous, some energy is not measured when the program performs other functions.
  This is compensated for with a scale factor applied to each energy measurement.
  These energy measurements are added to accumulate the total energy used. The time-averaged
  power is calculated by dividing the accumulated energy by the elapsed time.
 
  There are two current measurements made for lower and higher values. The low current range
  extends to 1 A with a resolution of about 1 ma. The higher range extends to 10 A with a
  resolution of 10 ma. Range switching is automatic. The overange for the voltage measurement
  is 27 V. The absolute maximum voltage at V+ or L+ is 40 V.
 
  A report is produced on a Sunfounder I2C LCD2004 display. The Sunfounder LCD library must
  be added to Arduino(#include <LiquidCrystal_I2C.h). In addition to the energy consumed and
  time-averaged power, the current and voltage at the time of the report is displayed. This
  and other iformation is also available via the usb port to a serial monitor.
 
 
*/
//LIBRARIES -- include the I2C and LCD library code
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4); // LCD address 0x27, 20 characters, 4 lines
/*********************************************************/
//GLOBAL VARIABLES
 
const int adcPinI1 = 0; // Low current measurement pin
const int adcPinV = 1; // Supply voltage measurement pin
const int adcPinI2 = 2; // High current measurement pin
const float rangeSW = .9; // Range switching threshold
const int inputPin = 5; // Run pushbutton input pin
float vRef = 1.097 ; // measured ADC reference voltage
float sfI1 = .8984 ; // current1 calibration scale-factor
float sfI2 = 9.891; // current1 calibration scale-factor
float sfV = 26.623 ; // voltage calibration scale-factor
float shunt = 0.0989; // measurement shunt value
float sfADC = 1023.0 / vRef; // scale factor for ADC measurements
float lossComp = 1.003 ; // initial compensation for time lost in other functions
float samTime;
static unsigned long beginRun = millis();
 
//FUNCTION DECLARATIONS
float sliceE();
void measureOnce();
void report();
 
/************************************************************/
void setup() {
 
  Serial.begin(9600);
  analogReference(INTERNAL);// use internal ADC ref ~ 1.1V
  pinMode(inputPin, INPUT_PULLUP);
 
  lcd.init();  //initialize the lcd
  lcd.backlight();  //open the backlight
 
  lcd.setCursor ( 0, 0 ); // go to the top left corner
  lcd.print("    PNSN mcEmeter   "); // write this string on the top row
  lcd.setCursor ( 0, 2 ); // go to the third row
  lcd.print("     Version 0.2");
  lcd.setCursor ( 0, 3 ); // go to the fourth row
  lcd.print("      Press Run");
 
  while ( digitalRead(inputPin) == HIGH) //Wait for Run button to start
  {
    delay(100);
  }
 
  measureOnce();
  //Serial.print("end setup");
}
/******************************************************/
void loop() {
 
 
  static float energyACC;
  static float wattHour, wattSecond, wattmilliSecond;
  static unsigned long beginMeasure = millis();
  long int duration; //
  float energySlice = 0;
  float eSlice = 0;
  unsigned long interval = millis();
  static  int blockN = 0;
 
  unsigned long beginSam = millis();
  long int samCount = 0;
 
sample:
  samCount = samCount + 1;
  energySlice = sliceE();
  eSlice = energySlice;
  energyACC = (float (energyACC + eSlice)); // uj accumulated
  if (energyACC > 1000000)
  {
    energyACC = energyACC - 1000000; //find remainder and reset
    wattSecond = wattSecond + 1; // joules accumulated
  }
 
 
  if (millis() - beginSam < 6000) // sets time between reports, 1 sec / 1000
  {
 
  samTime = float((millis() - beginSam) * 1000 / samCount); // average sample time, us
  goto sample;
  }
 
  else
  {
    unsigned long wasteStart = micros();
  blockN = blockN + 1;
  float wattSec = wattSecond + (energyACC / 1000000.00);
  float totWh = wattSec / 3600.00;
  float measure, waste;
    duration = (millis() - beginMeasure) / 1000; // seconds---float precision???
  float runTime = (duration / 3600.00); // convert time to hours
  float powAve = (totWh / (runTime));
  float avI1 = analogRead(adcPinI1);
  float avV = analogRead(adcPinV);
  float avI2 = analogRead(adcPinI2);
  float current1 = sfI1 * (avI1 / sfADC);
  float voltage = sfV * (avV / sfADC);
  float current2 = sfI2 * (avI2 / sfADC);
  float current = current1;
  if (current > rangeSW)
  {
      current = current2;
  }
  float difV = voltage - (current * shunt);
 
// Main  LCD report screen
    lcd.clear();
    lcd.setCursor ( 0, 0 ); // line 1
    lcd.print("Et="); // total energy
    lcd.print(totWh);
    lcd.print("Wh ");
    lcd.print("Pa="); // time averaged power
    lcd.print(powAve);
    lcd.print("W");
 
    lcd.setCursor ( 0, 1 ); // line 2
    lcd.print("VL=");
    lcd.print(difV); //instantaneous load voltage
    lcd.print("V ");
    lcd.print("IL=");
    lcd.print(current, 3); //instantaneous load current
    lcd.print("A");
 
  // lcd.setCursor ( 0, 2 ); // line 3
 
    lcd.setCursor ( 0, 3 );  // line 4
  lcd.print("Run=");
    lcd.print(runTime); // length of measurement run
    lcd.print("hr ");
    lcd.print("N=");
    lcd.print(blockN); // number of measurement reports
 
/************************************************************************************/
//If the serial monitor report below is enabled the meter will reset when the Arduino
//serial monitor is run.
/************************************************************************************/
 
    Serial.print(blockN);// the number of measurements reported
    Serial.print("  SamTime = ");// the average sample time
    Serial.print(samTime);
    Serial.print(" us   ");
    Serial.print("Loss Compensation = ");// Energy multiplier compensation time
    Serial.println(lossComp, 6);
    Serial.print("Momentary Readings   "); //status at time of report
    Serial.print("Run Time...");
    Serial.print(runTime, 4);
    Serial.println(" hrs   ");
    Serial.print(" Vload = ");
    Serial.print(difV, 4);
    Serial.print(" Vdc   ");  // last DUT voltage
    Serial.print(" I1 load = ");
    Serial.print(current1, 4);  // low range DUT current
    Serial.print(" Adc   ");
    Serial.print(" I2 load = ");
    Serial.print(current2, 4);  // high range DUT current
    Serial.print(" Adc   ");
    Serial.print(" Pload = ");
    Serial.print(current * difV); // instantaneous power
    Serial.println(" W   ");
    Serial.println("Long Term Values"); // on-going measurements
    Serial.print("Energy = ");
    Serial.print(totWh, 4); //
    Serial.print(" Wh   "); //
    Serial.print("Time averaged power = ");
    Serial.print(powAve, 4);
    Serial.println(" W");
    Serial.println("");
  
  measure = (millis() - interval); // time used in measurement
  waste = ((micros() - wasteStart) / 1000.0); // time not used in measurement
    lossComp = (measure + waste) / measure; //lost time compensation calc
  
  }
}
 
// Repeated energy measurement
float sliceE()
{
  float Iadc1 = 0;
  float Vadc = 0;
  float Iadc2 = 0;
  float energy;
  Iadc1 = analogRead(adcPinI1);
  Vadc = analogRead(adcPinV);
  Iadc2 = analogRead(adcPinI2);
 
  float current1 = sfI1 * (Iadc1 / sfADC);// low range current calc
  float voltage = sfV * (Vadc / sfADC); // voltage calc
  float current2 = sfI2 * (Iadc2 / sfADC);// high range current calc
  float current = current1;
  if (current > rangeSW) // current range switch
  {
  current = current2;
  }
  float difV = voltage - (current * shunt); // subtract the shunt IR for load voltage
  energy = float ( current * difV * samTime * lossComp); // E * compensation for time
 
  return energy;
}
 
// Provides initial measurement screen
void measureOnce()
{
  float avI1 = analogRead(adcPinI1);// wake the ADCs by reading channels
  float avI2 = analogRead(adcPinI2);
  float avV = analogRead(adcPinV);//
  delay(100);// yawn
  avI1 = analogRead(adcPinI1);// use these measurements
  avV = analogRead(adcPinV);
  avI2 = analogRead(adcPinI2);
 
  float current1 = sfI1 * (avI1 / sfADC);// low range current calc
  float voltage = sfV * (avV / sfADC); // voltage calc
  float current2 = sfI2 * (avI2 / sfADC);// high range current calc
  float current = current1;
  if (current > rangeSW) // current range switch
  {
  current = current2;
  }
  float difV = voltage - (current * shunt); // subtract the shunt IR for load voltage
 
// Initial report
  lcd.clear();
  lcd.setCursor ( 0, 0 );
  lcd.print("Starting values");
 
  lcd.setCursor ( 0, 1 );
  lcd.print("Vload = ");
  lcd.setCursor ( 8, 1 );
  lcd.print(difV);
  lcd.setCursor ( 13, 1 );
  lcd.print(" V");
 
  lcd.setCursor ( 0, 2 );
  lcd.print("Iload = ");
  lcd.setCursor ( 8, 2 );
  lcd.print(current);
  lcd.setCursor ( 13, 2 );
  lcd.print(" A");
 
  lcd.setCursor ( 0, 3 );
  lcd.print("Pload = ");
  lcd.setCursor ( 8, 3 );
  lcd.print(current * difV);
  lcd.setCursor ( 13, 3 );
  lcd.print(" W");
 }
 
// That's all folks!
