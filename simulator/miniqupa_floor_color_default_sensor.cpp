// Floor sampling follows the ARGoS ground-sensor pattern (Carlo Pinciroli).
// Preserve RGB rather than converting the floor color to reflectance/grayscale.
#include <argos3/plugins/robots/miniqupa/control_interface/ci_miniqupa_floor_color_sensor.h>
#include "miniqupa_floor_color_geometry.h"
#include <argos3/core/simulator/sensor.h>
#include <argos3/core/simulator/simulator.h>
#include <argos3/core/simulator/space/space.h>
#include <argos3/core/simulator/entity/composable_entity.h>
#include <argos3/core/simulator/entity/embodied_entity.h>
#include <argos3/core/simulator/entity/controllable_entity.h>
#include <argos3/core/simulator/entity/floor_entity.h>
#include <cmath>
namespace argos {
class CMiniQupaFloorColorDefaultSensor : public CSimulatedSensor,public CCI_MiniQupaFloorColorSensor {
 public:
  void SetRobot(CComposableEntity& entity) override {
   m_body=&entity.GetComponent<CEmbodiedEntity>("body");
   m_controller=&entity.GetComponent<CControllableEntity>("controller");
   m_floor=&CSimulator::GetInstance().GetSpace().GetFloorEntity();
  }
  void Init(TConfigurationNode& tree) override {
   GetNodeAttributeOrDefault(tree,"offset",m_offset,m_offset);
   GetNodeAttributeOrDefault(tree,"show_rays",m_show_rays,m_show_rays);
   if(!std::isfinite(m_offset.GetX())||!std::isfinite(m_offset.GetY())||!std::isfinite(m_offset.GetZ())||m_offset.GetZ()<0)
    THROW_ARGOSEXCEPTION("Invalid MiniQupa floor color offset");
   Reset();Enable();
  }
  void Update() override {
   if(IsDisabled())return;
   const auto& anchor=m_body->GetOriginAnchor();
   CVector3 origin=m_offset;origin.Rotate(anchor.Orientation);origin+=anchor.Position;
   // Planar model: a single floor point directly below the mounted sensor.
   m_reading.Color=m_floor->GetColorAtPoint(origin.GetX(),origin.GetY());m_reading.Valid=true;
   if(m_show_rays)m_controller->AddCheckedRay(true,CRay3(origin,CVector3(origin.GetX(),origin.GetY(),0)));
  }
  void Reset() override {m_reading=SReading{};}
 private:
  CEmbodiedEntity* m_body=nullptr;CControllableEntity* m_controller=nullptr;CFloorEntity* m_floor=nullptr;
  CVector3 m_offset{MiniQupaFloorColorOrigin[0],MiniQupaFloorColorOrigin[1],MiniQupaFloorColorOrigin[2]};
  bool m_show_rays=false;
};
REGISTER_SENSOR(CMiniQupaFloorColorDefaultSensor,"miniqupa_floor_color","default","MiniQupa workspace","0.1","Ideal downward RGB floor color.","Returns the RGB floor color below one CAD-based configurable offset. Intended for planar dynamics2d: no raw RGBC counts, illumination, integration time, spectral response, sampling footprint or occlusion model. Black is a valid floor color; validity is false after reset until the first update.","Usable");
}
