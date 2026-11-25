#pragma once
#include <argos3/core/control_interface/ci_sensor.h>
#include <argos3/core/utility/datatypes/color.h>
#include <argos3/core/utility/math/angles.h>
#include <vector>
namespace argos {
/** Ideal panoramic color segments. No robot identity or metric range is exposed. */
class CCI_MiniQupaCameraSensor : public CCI_Sensor {
 public:
  struct SBlob {
   CColor Color;
   CRadians Angle; // Local azimuth, positive left, [-pi,pi].
   CRadians AngularWidth;
   size_t Samples=0; // Diagnostic count of visible optical proxy points.
  };
  const std::vector<SBlob>& GetBlobs() const { return m_blobs; }
  size_t GetRawSampleCount() const { return m_raw_count; }
#ifdef ARGOS_WITH_LUA
  void CreateLuaState(lua_State* state) override;
  void ReadingsToLuaState(lua_State* state) override;
#endif
 protected:
  std::vector<SBlob> m_blobs;
  size_t m_raw_count=0;
};
}
