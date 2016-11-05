#include <Arduino.h>

class Barometer {
private:
  bool dataIsReady = false;
  float pressure;
  float temperature;
public:
  void adjustData();
  bool canShowData() { return dataIsReady; };

  String getPressure();
  String getTemperature();
};
