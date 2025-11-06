#include <Adafruit_MLX90614.h>

void setup() {
  // Set up for temp sensor
  Serial.begin(9600);
  while (!Serial);

  if (!mlx.begin()) {
    Serial.println("Error")
  };
}

float readTemp() {
  // need to include adafruit mlx90614 library
  
  // init temps; can reuse for analysis
  tempC = mlx.readObjectTempC();
  tempF = mlx.readObjectTempF();

  //prints values w/ 2 deci places
  sprintf(readingC, "Current Temp *C: %.2f%%", tempC);
  sprintf(readingF, "Current Temp *F: %.2f%%", tempF);
  Serial.println(readingC);
  Serial.println(readingF);
}