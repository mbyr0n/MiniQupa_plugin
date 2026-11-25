#include <argos3/plugins/robots/miniqupa/control_interface/ci_miniqupa_encoder_sensor.h>
#ifdef ARGOS_WITH_LUA
#include <argos3/core/wrappers/lua/lua_utility.h>
namespace argos {
namespace {
void WriteEncoder(lua_State* state,const CCI_MiniQupaEncoderSensor::SReading& reading) {
 CLuaUtility::AddToTable(state,"distance_left",reading.CoveredDistanceLeftWheel);
 CLuaUtility::AddToTable(state,"distance_right",reading.CoveredDistanceRightWheel);
 CLuaUtility::AddToTable(state,"axis_length",reading.WheelAxisLength);
}
}
void CCI_MiniQupaEncoderSensor::CreateLuaState(lua_State* state) {
 CLuaUtility::OpenRobotStateTable(state,"wheels");CLuaUtility::StartTable(state,"encoder");WriteEncoder(state,m_reading);CLuaUtility::EndTable(state);CLuaUtility::CloseRobotStateTable(state);
}
void CCI_MiniQupaEncoderSensor::ReadingsToLuaState(lua_State* state) {
 lua_getfield(state,-1,"wheels");lua_getfield(state,-1,"encoder");WriteEncoder(state,m_reading);lua_pop(state,2);
}
}
#endif
