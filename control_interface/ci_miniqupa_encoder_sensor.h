#pragma once
#include <argos3/core/control_interface/ci_sensor.h>
namespace argos {
/** Rolling-equivalent distances since the preceding sample, in meters.
 * Ideal no-slip odometry, not independently simulated motor-shaft rotation.
 */
class CCI_MiniQupaEncoderSensor : public CCI_Sensor {
 public:
  struct SReading {
   Real CoveredDistanceLeftWheel=0;
   Real CoveredDistanceRightWheel=0;
   Real WheelAxisLength=0;
  };
  const SReading& GetReading() const {return m_reading;}
#ifdef ARGOS_WITH_LUA
  void CreateLuaState(lua_State* state) override;
  void ReadingsToLuaState(lua_State* state) override;
#endif
 protected:
  SReading m_reading;
};
}
