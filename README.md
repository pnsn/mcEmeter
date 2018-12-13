# mcEmeter
  Measures the long term energy usage of a load by sampling its DC current and
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
  and other information is also available via the usb port to a serial monitor. 
