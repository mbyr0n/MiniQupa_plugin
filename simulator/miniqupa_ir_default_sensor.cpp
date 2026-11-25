// Raycast follows the ARGoS generic proximity sensor pattern (Carlo Pinciroli).
#include <argos3/plugins/robots/miniqupa/control_interface/ci_miniqupa_ir_sensor.h>
#include "miniqupa_ir_geometry.h"
#include <argos3/core/simulator/sensor.h>
#include <argos3/core/simulator/space/space.h>
#include <argos3/core/simulator/entity/composable_entity.h>
#include <argos3/core/simulator/entity/embodied_entity.h>
#include <argos3/core/simulator/entity/controllable_entity.h>
#include <argos3/plugins/simulator/entities/proximity_sensor_equipped_entity.h>
#include <cmath>
#include <algorithm>
#include <argos3/core/utility/math/rng.h>
namespace argos {
class CMiniQupaIRDefaultSensor : public CSimulatedSensor,public CCI_MiniQupaIRSensor {
 public:
  void SetRobot(CComposableEntity& entity) override {
   m_body=&entity.GetComponent<CEmbodiedEntity>("body");
   m_ir=&entity.GetComponent<CProximitySensorEquippedEntity>("proximity_sensors");
   m_controller=&entity.GetComponent<CControllableEntity>("controller");
   m_ir->Enable();
  }
  void Init(TConfigurationNode& tree) override {
   GetNodeAttributeOrDefault(tree,"show_rays",m_show_rays,m_show_rays);
   GetNodeAttributeOrDefault(tree,"min_range",m_min_range,m_min_range);
   GetNodeAttributeOrDefault(tree,"noise_level",m_noise_level,m_noise_level);
   if(!std::isfinite(m_min_range) || m_min_range<=0 || !std::isfinite(m_noise_level) || m_noise_level<0)
    THROW_ARGOSEXCEPTION("min_range must be finite and positive; noise_level finite and nonnegative");
   if(m_noise_level>0) m_rng=CRandom::CreateRNG("argos");
   if(m_ir->GetNumSensors()!=MiniQupaIRPoses.size()) THROW_ARGOSEXCEPTION("MiniQupa requires exactly eight IRs");
   m_readings.resize(m_ir->GetNumSensors());
   for(size_t i=0;i<m_readings.size();++i) {
    auto& r=m_readings[i];r.Id=MiniQupaIRPoses[i].Id;r.Range=m_ir->GetSensor(i).Direction.Length();
    if(m_min_range>=r.Range) THROW_ARGOSEXCEPTION("min_range must be smaller than ir_range");
    r.Angle=CRadians(std::atan2(MiniQupaIRPoses[i].DY,MiniQupaIRPoses[i].DX));
   }
   Reset();Enable();
  }
  void Update() override {
   if(IsDisabled()) return;
   for(size_t i=0;i<m_readings.size();++i) {
    const auto& sensor=m_ir->GetSensor(i);
    CVector3 start=sensor.Offset,end=sensor.Offset+sensor.Direction;
    start.Rotate(sensor.Anchor.Orientation);start+=sensor.Anchor.Position;
    end.Rotate(sensor.Anchor.Orientation);end+=sensor.Anchor.Position;
    const CRay3 ray(start,end);
    SEmbodiedEntityIntersectionItem intersection;
    auto& r=m_readings[i];
    // Explicitly ignore this robot's conservative collision cylinder. The CAD
    // lens points lie inside that cylinder; it does not model the IR windows.
    r.Detected=GetClosestEmbodiedEntityIntersectedByRay(intersection,ray,*m_body);
    r.Distance=r.Detected?ray.GetDistance(intersection.TOnRay):r.Range;
    // Foot-bot response, rescaled so its saturation point corresponds to 4 cm
    // (or the configured min_range). This is a simulation choice, not calibration.
    if(!r.Detected) r.Value=0;
    else if(r.Distance<=m_min_range) r.Value=1;
    else {
     const Real scaled_distance=r.Distance*(0.009889556/m_min_range);
     r.Value=0.0100527/(scaled_distance+0.000163144);
    }
    if(m_rng) r.Value+=m_rng->Uniform(CRange<Real>(-m_noise_level,m_noise_level));
    r.Value=std::max(Real(0),std::min(Real(1),r.Value));
    if(m_show_rays) {
     m_controller->AddCheckedRay(r.Detected,ray);
     if(r.Detected) m_controller->AddIntersectionPoint(ray,intersection.TOnRay);
    }
   }
  }
  void Reset() override { for(auto& r:m_readings) {r.Detected=false;r.Distance=r.Range;r.Value=0;} }
 private:
  CEmbodiedEntity* m_body=nullptr;
  CProximitySensorEquippedEntity* m_ir=nullptr;
  CControllableEntity* m_controller=nullptr;
  bool m_show_rays=false;
  Real m_min_range=.04,m_noise_level=0;
  CRandom::CRNG* m_rng=nullptr;
};
REGISTER_SENSOR(CMiniQupaIRDefaultSensor,"miniqupa_ir","default","MiniQupa workspace","0.3","Eight normalized MiniQupa IR proximity readings.","Foot-bot-style response rescaled to min_range (default 0.04 m); saturation at 1, no hit at 0. Robot ir_range defaults to 0.40 m. Optional uniform noise_level and clipping to [0,1]. Metric distance and detected flag are ideal diagnostics, not real sensor outputs.","Usable");
}
