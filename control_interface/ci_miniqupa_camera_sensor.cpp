#include <argos3/plugins/robots/miniqupa/control_interface/ci_miniqupa_camera_sensor.h>
#ifdef ARGOS_WITH_LUA
#include <argos3/core/wrappers/lua/lua_utility.h>
namespace argos {
namespace {
void WriteCamera(lua_State* state,const std::vector<CCI_MiniQupaCameraSensor::SBlob>& blobs,size_t raw_count) {
 CLuaUtility::AddToTable(state,"raw_samples",static_cast<UInt32>(raw_count));
 lua_newtable(state);
 for(size_t i=0;i<blobs.size();++i) {
  const auto& b=blobs[i];lua_newtable(state);
  CLuaUtility::AddToTable(state,"color",b.Color);
  CLuaUtility::AddToTable(state,"angle",b.Angle.GetValue());
  CLuaUtility::AddToTable(state,"angular_width",b.AngularWidth.GetValue());
  CLuaUtility::AddToTable(state,"samples",static_cast<UInt32>(b.Samples));
  lua_rawseti(state,-2,i+1);
 }
 lua_setfield(state,-2,"blobs");
}
}
void CCI_MiniQupaCameraSensor::CreateLuaState(lua_State* state) {
 CLuaUtility::OpenRobotStateTable(state,"camera");WriteCamera(state,m_blobs,m_raw_count);CLuaUtility::CloseRobotStateTable(state);
}
void CCI_MiniQupaCameraSensor::ReadingsToLuaState(lua_State* state) {
 lua_getfield(state,-1,"camera");WriteCamera(state,m_blobs,m_raw_count);lua_pop(state,1);
}
}
#endif
