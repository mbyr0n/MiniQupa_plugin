#include <argos3/plugins/robots/miniqupa/control_interface/ci_miniqupa_floor_color_sensor.h>
#ifdef ARGOS_WITH_LUA
#include <argos3/core/wrappers/lua/lua_utility.h>
namespace argos {
namespace {
void WriteFloorColor(lua_State* state,const CCI_MiniQupaFloorColorSensor::SReading& reading) {
 CLuaUtility::AddToTable(state,"color",reading.Color);
 lua_pushboolean(state,reading.Valid);lua_setfield(state,-2,"valid");
}
}
void CCI_MiniQupaFloorColorSensor::CreateLuaState(lua_State* state) {
 CLuaUtility::OpenRobotStateTable(state,"floor_color");WriteFloorColor(state,m_reading);CLuaUtility::CloseRobotStateTable(state);
}
void CCI_MiniQupaFloorColorSensor::ReadingsToLuaState(lua_State* state) {
 lua_getfield(state,-1,"floor_color");WriteFloorColor(state,m_reading);lua_pop(state,1);
}
}
#endif
