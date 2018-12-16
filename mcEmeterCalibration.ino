/* Calibration routine measures voltage and currrent for comparison against values
    measured by other instruments so scale factors, sfV and sfI, can be determined
    to produce agreement. An accurate value of the board current measuring resistor
    and the Arduino 1.1V ADC reference should be entered under GLOBAL VARIABLES.

*/
//GLOBAL VARIABLES
const int adcPinI1 = 0; // current shunt measurement
const int adcPinV = 1;// supply voltage measurement
const int adcPinI2 = 2;
float vRef = 1.100 ; // measured ADC reference voltage for Uno 3
float sfV = 25.0 ; // Input voltage calibration scale-factor (Vin/Vout)
float sfI1 = 1.00 ; // Input current calibration scale-factor (Ain/Vout)
float sfI2 = 10.00 ; // Input current calibration scale-factor (Ain/Vout)
float shunt = 0.1 ; // measurement shunt value (0 for probe)
float sfADC = 1023.0 / vRef; // scale factor for ADC measurements

//FUNCTION DECLARATIONS
float samI();
float samV();

void setup() {  

  Serial.begin(9600);
  analogReference(INTERNAL);
}

void loop() {
  for (int i = 0; i < 1000; i ++)
  {
    float avI1 = samI1(); // call I1 adc  sample block
    float avV = samV(); // call V adc  sample block
    float avI2 = samI2(); // call I2 adc  sample block
    float current1 = sfI1 * (avI1 / sfADC);  //convert ADC to average current1
    float current2 = sfI2 * (avI2 / sfADC); // convert ADC to average current2
    float voltage = sfV * (avV / sfADC);  //convert ADC to average voltage
    float difV = voltage - (current1 * shunt); //correct for Vshunt
    float power = current1 * difV; // calculate power

    Serial.print("Iload1 = ");
    Serial.print(current1,4);
    Serial.print(" A     ");
    Serial.print("Iload2 = ");
    Serial.print(current2,4);
    Serial.print(" A     ");
    Serial.print("Vload = ");
    Serial.print(difV, 4);
    Serial.print(" V     ");
    Serial.print("Pload = ");
    Serial.print(power,4);
    Serial.println(" W");
    Serial.println(" ");
    delay(2000);

  }
}

float samI1() // takes 256 current samples and averages them
{
  long int accreadI = 0;
  float avI1;
  int i = 0;
  do
  {
    int readI = analogRead(adcPinI1);
    accreadI = accreadI + readI;
    i++;
  }
  while (i < 256);
  avI1 = accreadI / 256.0;
  return (avI1);  // returns the average of 256 samples
}
float samI2() // takes 256 current samples and averages them
{
  long int accreadI = 0;
  float avI2;
  int i = 0;
  do
  {
    int readI = analogRead(adcPinI2);
    accreadI = accreadI + readI;
    i++;
  }
  while (i < 256);
  avI2 = accreadI / 256.0;
  return (avI2);  // returns the average of 256 samples
}
float samV() // takes 256 supply voltage samples and averages them
{
  long  int accreadV = 0;
  float avV;
  int i = 0;
  do
  {
    int readV = analogRead(adcPinV);
    accreadV = accreadV + readV;
    i++;
  }
  while (i < 256);
  avV = accreadV / 256.0;
  return (avV);  // returns the average of 256 samples
}
