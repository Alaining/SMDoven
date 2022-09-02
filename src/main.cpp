/*
800W Reflow oven controller firmware
Using Arduino Nano 33 BLE sense and TL082 opamp
Programmed using VSCode with PlatformIO
*/

#include <Arduino_HTS221.h>
#include <CircularBuffer.h>
CircularBuffer<float, 3> raw; // buffer for RAW temperature readings
CircularBuffer<float, 3> iir; // buffer for IIR filtered readings

float b[4] = {1.0, 2.0, 1.0}; // Butterworth 0.1 to 20Hz
float a[4] = {1.0, -1.97945, 0.97966};

float old_temp = 0;
float old_hum = 0;

int sensorPin = A0;
int ticktime = 0;

void setup()
{
  delay(100);
  pinMode(D10, OUTPUT);
  digitalWrite(D10, LOW);
  analogReadResolution(12);
  Serial.begin(115200);
  while (!Serial)
    ;

  if (!HTS.begin())
  {
    Serial.println("Failed to initialize humidity temperature sensor!");
    while (1)
      ;
  }
  float x0 = HTS.readTemperature(); // Initial conditions of IIR filter = arduino_temp_sensor
  raw.unshift(x0);
  raw.unshift(x0);
  raw.unshift(x0);

  iir.unshift(x0);
  iir.unshift(x0);
  iir.unshift(x0);
}

void loop()
{
  // read all the sensor values
  float temperature = HTS.readTemperature();
  // float humidity = HTS.readHumidity();
  int sensorValue = analogRead(sensorPin);    // read analog pin (opamp output)
  float mvolts = (sensorValue / 1.241) - 532; // mvolts = analogRead_milivolts - (opamp offset)
  float cdeg = mvolts * 0.25 + temperature;   // cdeg = hot joint + cold joint (arduino temp) | RAW temp

  raw.unshift(cdeg);         // inset cdeg in circular buffer
  Serial.print(raw.first()); // print current RAW temp
  Serial.print("\t");
  float xn = raw[0];                                                                             // x(n)    | current input temp
  float xnm1 = raw[1];                                                                           // x(n-1)  | input temp 1 step before
  float xnm2 = raw[2];                                                                           // x(n-2)  | input temp 2 step before
  float ynm1 = iir[0];                                                                           // y(n)    | IIR output temp 1 step before
  float ynm2 = iir[1];                                                                           // y(n-1)  | IIR output temp 2 step before
  float iir_y = 0.0024925 * xn + 0.004985 * xnm1 + 0.0024925 * xnm2 + 1.92 * ynm1 - 0.93 * ynm2; // IIR filter | y(n)=...
  iir.unshift(iir_y);                                                                            // insert iir_y in circular buffer

  Serial.println(iir.first()); // print current IIR filtered temp
  Serial.println();

  if (iir.first() < 30 && ticktime > 20)
  {
    digitalWrite(D10, HIGH); //
  }

  if (iir.first() > 40 && ticktime > 20)
  {
    digitalWrite(D10, LOW); //
  }
  ticktime++;
  delay(100);
}