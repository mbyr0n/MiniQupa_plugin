#include <argos3/plugins/robots/miniqupa/control_interface/ci_miniqupa_encoder_sensor.h>
#include <argos3/plugins/robots/miniqupa/simulator/miniqupa_entity.h>
#include <argos3/core/simulator/sensor.h>
namespace argos {
class CMiniQupaEncoderDefaultSensor : public CSimulatedSensor,public CCI_MiniQupaEncoderSensor {
 public:
  void SetRobot(CComposableEntity& entity) override {m_robot=&dynamic_cast<CMiniQupaEntity&>(entity);}
  void Init(TConfigurationNode&) override {Reset();Enable();}
  void Update() override {
   if(IsDisabled())return;
   const auto& current=m_robot->GetEncoderDistances();
   m_reading.CoveredDistanceLeftWheel=current[0]-m_previous[0];
   m_reading.CoveredDistanceRightWheel=current[1]-m_previous[1];
   m_previous=current;
  }
  void Reset() override {
   m_reading={0,0,CMiniQupaEntity::Track};m_previous=m_robot->GetEncoderDistances();
  }
 private:
  CMiniQupaEntity* m_robot=nullptr;std::array<Real,2> m_previous{};
};
REGISTER_SENSOR(CMiniQupaEncoderDefaultSensor,"miniqupa_encoder","default","MiniQupa workspace","0.1","Ideal no-slip wheel encoder in meters.","Returns signed rolling-equivalent left/right wheel distances since the preceding sample and wheel-axis length, in meters. Derived from actual planar physics substeps rather than motor commands. Not independent shaft rotation, slip, stall or pulse-count simulation. Lua: robot.wheels.encoder.","Usable");
}
