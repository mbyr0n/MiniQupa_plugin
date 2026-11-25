#include <argos3/plugins/robots/miniqupa/simulator/miniqupa_entity.h>
#include "miniqupa_ir_geometry.h"
#include "miniqupa_tag_geometry.h"
#include <argos3/plugins/robots/miniqupa/simulator/miniqupa_tag_pattern.h>
#include <argos3/plugins/simulator/media/tag_medium.h>
#include <cmath>
#include <argos3/plugins/simulator/media/simple_radio_medium.h>
#include <argos3/core/simulator/simulator.h>
#include <argos3/core/simulator/space/space.h>
namespace argos {
void CMiniQupaEntity::Init(TConfigurationNode& tree) {
 try {
  CComposableEntity::Init(tree);
  GetNodeAttributeOrDefault(tree,"mass",m_mass,m_mass);
  GetNodeAttributeOrDefault(tree,"friction",m_friction,m_friction);
  GetNodeAttributeOrDefault(tree,"max_force",m_force,m_force);
  GetNodeAttributeOrDefault(tree,"max_torque",m_torque,m_torque);
  if(m_mass<=0 || m_friction<0 || m_force<=0 || m_torque<=0)
   THROW_ARGOSEXCEPTION("Invalid MiniQupa physics parameters");
  m_body = new CEmbodiedEntity(this);
  AddComponent(*m_body);
  m_body->Init(GetNode(tree,"body"));
  m_wheels = new CWheeledEntity(this,"wheels_0",2);
  AddComponent(*m_wheels);
  // Coplanar effective axle for differential kinematics; CAD asymmetry is <0.1 mm.
  m_wheels->SetWheel(0,CVector3(0,Track*.5,WheelRadius),WheelRadius);
  m_wheels->SetWheel(1,CVector3(0,-Track*.5,WheelRadius),WheelRadius);
  GetNodeAttributeOrDefault(tree,"ir_range",m_ir_range,m_ir_range);
  if(!std::isfinite(m_ir_range) || m_ir_range<=0) THROW_ARGOSEXCEPTION("ir_range must be finite and positive");
  m_ir = new CProximitySensorEquippedEntity(this,"proximity_0");
  AddComponent(*m_ir);
  for(const auto& pose:MiniQupaIRPoses) {
   m_ir->AddSensor(CVector3(pose.X,pose.Y,pose.Z),CVector3(pose.DX,pose.DY,pose.DZ),m_ir_range,m_body->GetOriginAnchor());
  }
  m_leds = new CLEDEquippedEntity(this,"leds_0");
  AddComponent(*m_leds);
  // Optical surface samples, not assumed positions of the five internal LEDs.
  // Just outside the conservative collision radius so other cameras can see them.
  m_leds->AddLEDRing(CVector3(0,0,DiffuserSampleHeight),DiffuserSampleRadius,CRadians::ZERO,DiffuserSamples,m_body->GetOriginAnchor());
  InitRadios(tree);
  InitRangeAndBearing(tree);
  InitTags(tree);
  m_controller = new CControllableEntity(this);
  AddComponent(*m_controller);
  m_controller->Init(GetNode(tree,"controller"));
  UpdateComponents();
 } catch(CARGoSException& ex) {
  THROW_ARGOSEXCEPTION_NESTED("Cannot initialize MiniQupa " << GetId(),ex);
 }
}
void CMiniQupaEntity::InitRadios(TConfigurationNode& tree) {
 m_radios=new CSimpleRadioEquippedEntity(this,"simple_radios_0");AddComponent(*m_radios);
 std::string medium;GetNodeAttributeOrDefault(tree,"radio_medium",medium,medium);
 Real range=1.0;CVector3 position;
 GetNodeAttributeOrDefault(tree,"radio_range",range,range);
 GetNodeAttributeOrDefault(tree,"radio_position",position,position);
 if(!std::isfinite(range)||range<=0||!std::isfinite(position.GetX())||!std::isfinite(position.GetY())||!std::isfinite(position.GetZ()))
  THROW_ARGOSEXCEPTION("Invalid MiniQupa radio range or position");
 if(medium.empty())return; // Optional: existing experiments require no radio medium.
 m_radios->AddRadio("espnow",position,m_body->GetOriginAnchor(),CSimulator::GetInstance().GetMedium<CSimpleRadioMedium>(medium),range);
 m_radios->Enable();
}
void CMiniQupaEntity::InitRangeAndBearing(TConfigurationNode& tree) {
 Real range=3.0;SInt32 size=10;CVector3 position(0,0,.1);
 GetNodeAttributeOrDefault(tree,"rab_range",range,range);
 GetNodeAttributeOrDefault(tree,"rab_data_size",size,size);
 GetNodeAttributeOrDefault(tree,"rab_position",position,position);
 if(!std::isfinite(range)||range<=0||size<=0||!std::isfinite(position.GetX())||!std::isfinite(position.GetY())||!std::isfinite(position.GetZ()))
  THROW_ARGOSEXCEPTION("Invalid MiniQupa RAB parameters");
 // Native foot-bot defaults; position is a virtual reference, not measured hardware.
 m_rab=new CRABEquippedEntity(this,"rab_0",size,range,m_body->GetOriginAnchor(),*m_body,position);
 AddComponent(*m_rab); // Disabled until a native RAB sensor or actuator enables it.
}
void CMiniQupaEntity::InitTags(TConfigurationNode& tree) {
 m_tags=new CTagEquippedEntity(this,"tags_0");AddComponent(*m_tags);
 if(!NodeAttributeExists(tree,"tag_id"))return;
 std::string payload;GetNodeAttribute(tree,"tag_id",payload);std::uint32_t id=0;
 if(!MiniQupaParseTagId(payload,id))THROW_ARGOSEXCEPTION("Invalid MiniQupa tag ID: use an integer 0..4294967295");
 Real side=MiniQupaTagSide,yaw=0;
 CVector3 position(MiniQupaTagPosition[0],MiniQupaTagPosition[1],MiniQupaTagPosition[2]);
 GetNodeAttributeOrDefault(tree,"tag_side_length",side,side);GetNodeAttributeOrDefault(tree,"tag_position",position,position);GetNodeAttributeOrDefault(tree,"tag_yaw",yaw,yaw);
 if(!std::isfinite(side)||side<=0||!std::isfinite(yaw)||!std::isfinite(position.GetX())||!std::isfinite(position.GetY())||!std::isfinite(position.GetZ()))
  THROW_ARGOSEXCEPTION("Invalid MiniQupa tag geometry");
 std::string medium="tags";GetNodeAttributeOrDefault(tree,"tag_medium",medium,medium);
 m_tags->AddTag("top",position,CQuaternion(CRadians(yaw*ARGOS_PI/180),CVector3::Z),m_body->GetOriginAnchor(),CRadians(ARGOS_PI/3),side,std::to_string(id));
 m_tags->SetMedium(CSimulator::GetInstance().GetMedium<CTagMedium>(medium));m_tags->Enable();
}
void CMiniQupaEntity::SetDiffuserColor(const CColor& color) {
 m_diffuser_color=color;
 if(m_leds) m_leds->SetAllLEDsColors(color);
}
void CMiniQupaEntity::UpdateComponents() {
 if(m_leds && m_leds->IsEnabled()) m_leds->Update();
 if(m_radios && m_radios->IsEnabled()) m_radios->Update();
 if(m_rab && m_rab->IsEnabled()) m_rab->Update();
 if(m_tags && m_tags->IsEnabled()) m_tags->Update();
}
REGISTER_ENTITY(CMiniQupaEntity,"miniqupa","MiniQupa workspace","0.2","MiniQupa differential robot.","CAD-based differential robot with eight IR sensors, downward RGB, panoramic blobs, common RGB diffuser, ideal rolling encoders and optional native tags. Supports native simple_radios and range-and-bearing. Configure devices in the controller and matching media in the experiment.","Usable");
REGISTER_STANDARD_SPACE_OPERATIONS_ON_COMPOSABLE(CMiniQupaEntity);
}
