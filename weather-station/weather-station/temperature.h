#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>

class Temperature {
private:
  Adafruit_BMP085_Unified bmpSensor;
  bool dataIsReady = false;
  float pressure;
  float temperature;
public:
  Temperature();
  void adjustData();
  bool canShowData();

  String pressure();
  String temperature();
};
