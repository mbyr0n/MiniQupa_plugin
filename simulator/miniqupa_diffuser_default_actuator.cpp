#include <argos3/plugins/robots/miniqupa/control_interface/ci_miniqupa_diffuser_actuator.h>
#include <argos3/plugins/robots/miniqupa/simulator/miniqupa_entity.h>
#include <argos3/core/simulator/actuator.h>
#include <argos3/core/simulator/simulator.h>
#include <argos3/plugins/simulator/media/led_medium.h>
#ifdef ARGOS_WITH_LUA
#include <argos3/core/wrappers/lua/lua_utility.h>
#endif
namespace argos {
#ifdef ARGOS_WITH_LUA
namespace {
int SetDiffuserColor(lua_State* state) {
 if(lua_gettop(state)!=3 || !lua_isnumber(state,1) || !lua_isnumber(state,2) || !lua_isnumber(state,3))
  return luaL_error(state,"diffuser.set_color expects three integer RGB channels, 0..255");
 for(int i=1;i<=3;++i) {
  const lua_Number n=lua_tonumber(state,i);
  if(n<0 || n>255 || n!=lua_tointeger(state,i)) return luaL_error(state,"RGB channels must be integers in 0..255");
 }
 CLuaUtility::GetDeviceInstance<CCI_MiniQupaDiffuserActuator>(state,"diffuser")->SetColor(
  CColor(lua_tointeger(state,1),lua_tointeger(state,2),lua_tointeger(state,3)));
 return 0;
}
}
void CCI_MiniQupaDiffuserActuator::CreateLuaState(lua_State* state) {
 CLuaUtility::OpenRobotStateTable(state,"diffuser");
 CLuaUtility::AddToTable(state,"_instance",this);
 CLuaUtility::AddToTable(state,"set_color",&SetDiffuserColor);
 CLuaUtility::CloseRobotStateTable(state);
}
#endif
class CMiniQupaDiffuserDefaultActuator : public CSimulatedActuator, public CCI_MiniQupaDiffuserActuator {
 public:
  void SetRobot(CComposableEntity& entity) override { m_robot=&dynamic_cast<CMiniQupaEntity&>(entity); }
  void Init(TConfigurationNode& tree) override {
   std::string medium="leds";GetNodeAttributeOrDefault(tree,"medium",medium,medium);
   auto& leds=m_robot->GetLEDEquippedEntity();
   leds.SetMedium(CSimulator::GetInstance().GetMedium<CLEDMedium>(medium));leds.Enable();
  }
  void Update() override { m_robot->SetDiffuserColor(m_color); }
  void Reset() override {TurnOff();m_robot->SetDiffuserColor(CColor::BLACK);}
  void Destroy() override {m_robot->SetDiffuserColor(CColor::BLACK);m_robot->GetLEDEquippedEntity().Disable();}
 private:
  CMiniQupaEntity* m_robot=nullptr;
};
REGISTER_ACTUATOR(CMiniQupaDiffuserDefaultActuator,"miniqupa_diffuser","default","MiniQupa workspace","0.1","Common RGB control of the MiniQupa diffuser.","One RGB command controls the whole diffuser and its optical proxy emitters. Requires an LED medium. BLACK switches emission off; it does not paint the unlit diffuser black.","Usable");
}
