#include <Arduino.h>

class Temperature {
private:
  bool dataIsReady;
  float pressure;
  float temperature;
public:
  Temperature();
  void adjustData();
  bool canShowData();

  String getPressure();
  String getTemperature();
};
