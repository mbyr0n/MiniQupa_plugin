#pragma once
#include <argos3/core/control_interface/ci_actuator.h>
#include <argos3/core/utility/datatypes/color.h>
namespace argos {
/** One common RGB command for the five physical LEDs illuminating the diffuser. */
class CCI_MiniQupaDiffuserActuator : public CCI_Actuator {
 public:
  void SetColor(const CColor& color) { m_color=color; }
  void TurnOff() { m_color=CColor::BLACK; }
#ifdef ARGOS_WITH_LUA
  void CreateLuaState(lua_State* state) override;
#endif
 protected:
  CColor m_color=CColor::BLACK;
};
}
