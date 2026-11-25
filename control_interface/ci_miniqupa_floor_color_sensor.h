#pragma once
#include <argos3/core/control_interface/ci_sensor.h>
#include <argos3/core/utility/datatypes/color.h>
namespace argos {
/** Ideal RGB at one ground point; not raw TCS34725 RGBC counts. */
class CCI_MiniQupaFloorColorSensor : public CCI_Sensor {
 public:
  struct SReading { CColor Color=CColor::BLACK;bool Valid=false; };
  const SReading& GetReading() const {return m_reading;}
#ifdef ARGOS_WITH_LUA
  void CreateLuaState(lua_State* state) override;
  void ReadingsToLuaState(lua_State* state) override;
#endif
 protected:
  SReading m_reading;
};
}
