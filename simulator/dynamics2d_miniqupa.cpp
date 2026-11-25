// Differential steering structure adapted from ARGoS e-puck (Carlo Pinciroli).
#include <argos3/plugins/robots/miniqupa/simulator/miniqupa_entity.h>
#include <argos3/plugins/robots/miniqupa/simulator/miniqupa_encoder_kinematics.h>
#include <argos3/plugins/simulator/physics_engines/dynamics2d/dynamics2d_single_body_object_model.h>
#include <argos3/plugins/simulator/physics_engines/dynamics2d/dynamics2d_differentialsteering_control.h>
#include <argos3/plugins/simulator/physics_engines/dynamics2d/dynamics2d_engine.h>
namespace argos {
class CDynamics2DMiniQupa : public CDynamics2DSingleBodyObjectModel {
 public:
  CDynamics2DMiniQupa(CDynamics2DEngine& engine,CMiniQupaEntity& entity) :
   CDynamics2DSingleBodyObjectModel(engine,entity),m_entity(entity),
   m_steering(engine,entity.MaxForce(),entity.MaxTorque(),CMiniQupaEntity::Track,entity.GetConfigurationNode()),
   m_velocity(entity.GetWheeledEntity().GetWheelVelocities()) {
   cpBody* body=cpSpaceAddBody(engine.GetPhysicsSpace(),cpBodyNew(entity.Mass(),cpMomentForCircle(entity.Mass(),0,CMiniQupaEntity::Radius,cpvzero)));
   const auto& anchor=entity.GetEmbodiedEntity().GetOriginAnchor();
   body->p=cpv(anchor.Position.GetX(),anchor.Position.GetY());
   CRadians z,y,x;anchor.Orientation.ToEulerAngles(z,y,x);cpBodySetAngle(body,z.GetValue());
   cpShape* shape=cpSpaceAddShape(engine.GetPhysicsSpace(),cpCircleShapeNew(body,CMiniQupaEntity::Radius,cpvzero));
   shape->e=0;shape->u=entity.Friction();
   m_steering.AttachTo(body);SetBody(body,CMiniQupaEntity::Height);ResetEncoderBaseline();
  }
  ~CDynamics2DMiniQupa() override { m_steering.Detach(); }
  void Reset() override { CDynamics2DSingleBodyObjectModel::Reset();m_steering.Reset();ResetEncoderBaseline(); }
  void UpdatePhysics() override {RecordEncoderMotion();}
  void UpdateEntityStatus() override {
   RecordEncoderMotion();CDynamics2DSingleBodyObjectModel::UpdateEntityStatus();
  }
  void MoveTo(const CVector3& position,const CQuaternion& orientation) override {
   CDynamics2DSingleBodyObjectModel::MoveTo(position,orientation);ResetEncoderBaseline();
  }
  void UpdateFromEntityStatus() override {
   if(m_velocity[0]!=0 || m_velocity[1]!=0) m_steering.SetWheelVelocity(m_velocity[0],m_velocity[1]);
   else m_steering.Reset();
  }
 private:
  void ResetEncoderBaseline() {
   const auto* body=m_steering.GetControlledBody();m_previous_position=body->p;m_previous_angle=body->a;
  }
  void RecordEncoderMotion() {
   const auto* body=m_steering.GetControlledBody();
   const auto delta=MiniQupaRollingIncrement(body->p.x-m_previous_position.x,body->p.y-m_previous_position.y,m_previous_angle,body->a,CMiniQupaEntity::Track);
   m_entity.AccumulateEncoderDistances(delta[0],delta[1]);ResetEncoderBaseline();
  }
  CMiniQupaEntity& m_entity;
  CDynamics2DDifferentialSteeringControl m_steering;
  const Real* m_velocity;
  cpVect m_previous_position=cpvzero;Real m_previous_angle=0;
};
REGISTER_STANDARD_DYNAMICS2D_OPERATIONS_ON_ENTITY(CMiniQupaEntity,CDynamics2DMiniQupa);
}
