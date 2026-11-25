#pragma once
#include <argos3/core/control_interface/ci_sensor.h>
#include <argos3/core/utility/math/angles.h>
#include <string>
#include <vector>
namespace argos {
/** Normalized foot-bot-style proximity, with metric diagnostics.
 * No detection is represented explicitly by Detected=false and Distance=Range.
 * IR IDs are CAD review labels; the firmware index mapping is not yet known.
 */
class CCI_MiniQupaIRSensor : public CCI_Sensor {
 public:
  struct SReading {
   std::string Id;
   Real Value = 0; // 0: no obstacle; 1: saturated proximity.
   CRadians Angle; // Local yaw in radians, positive to the left.
   Real Distance = 0;
   Real Range = 0;
   bool Detected = false;
  };
  const std::vector<SReading>& GetReadings() const { return m_readings; }
#ifdef ARGOS_WITH_LUA
  void CreateLuaState(lua_State* state) override;
  void ReadingsToLuaState(lua_State* state) override;
#endif
 protected:
  std::vector<SReading> m_readings;
};
}
