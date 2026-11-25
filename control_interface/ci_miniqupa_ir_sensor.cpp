#include <argos3/plugins/robots/miniqupa/control_interface/ci_miniqupa_ir_sensor.h>
#ifdef ARGOS_WITH_LUA
#include <argos3/core/wrappers/lua/lua_utility.h>
namespace argos {
namespace {
void WriteReadings(lua_State* state,const std::vector<CCI_MiniQupaIRSensor::SReading>& readings) {
 for(size_t i=0;i<readings.size();++i) {
  const auto& r=readings[i];
  lua_newtable(state);
  CLuaUtility::AddToTable(state,"id",r.Id);
  CLuaUtility::AddToTable(state,"value",r.Value);
  CLuaUtility::AddToTable(state,"angle",r.Angle.GetValue());
  CLuaUtility::AddToTable(state,"distance_m",r.Distance);
  CLuaUtility::AddToTable(state,"range_m",r.Range);
  lua_pushboolean(state,r.Detected);lua_setfield(state,-2,"detected");
  lua_rawseti(state,-2,i+1);
 }
}
}
void CCI_MiniQupaIRSensor::CreateLuaState(lua_State* state) {
 CLuaUtility::OpenRobotStateTable(state,"miniqupa_ir");
 WriteReadings(state,m_readings);
 CLuaUtility::CloseRobotStateTable(state);
}
void CCI_MiniQupaIRSensor::ReadingsToLuaState(lua_State* state) {
 lua_getfield(state,-1,"miniqupa_ir");WriteReadings(state,m_readings);lua_pop(state,1);
}
}
#endif
