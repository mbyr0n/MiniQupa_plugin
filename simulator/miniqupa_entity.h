#pragma once
#include <array>
#include <argos3/core/simulator/entity/composable_entity.h>
#include <argos3/core/simulator/entity/embodied_entity.h>
#include <argos3/core/simulator/entity/controllable_entity.h>
#include <argos3/plugins/simulator/entities/wheeled_entity.h>
#include <argos3/plugins/simulator/entities/simple_radio_equipped_entity.h>
#include <argos3/plugins/simulator/entities/rab_equipped_entity.h>
#include <argos3/plugins/simulator/entities/tag_equipped_entity.h>
#include <argos3/plugins/simulator/entities/led_equipped_entity.h>
#include <argos3/plugins/simulator/entities/proximity_sensor_equipped_entity.h>
namespace argos {
class CMiniQupaEntity : public CComposableEntity {
 public:
  ENABLE_VTABLE();
  CMiniQupaEntity() : CComposableEntity(nullptr) {}
  void Init(TConfigurationNode& tree) override;
  void Reset() override { m_encoder_distances={0,0};CComposableEntity::Reset();SetDiffuserColor(CColor::BLACK);UpdateComponents(); }
  void UpdateComponents() override;
  const CTagEquippedEntity& GetTagEquippedEntity() const {return *m_tags;}
  CLEDEquippedEntity& GetLEDEquippedEntity() { return *m_leds; }
  const CColor& GetDiffuserColor() const { return m_diffuser_color; }
  void SetDiffuserColor(const CColor& color);
  static constexpr size_t DiffuserSamples=16;
  static constexpr Real DiffuserSampleRadius=.0544;
  static constexpr Real DiffuserSampleHeight=.119225;
  std::string GetTypeDescription() const override { return "miniqupa"; }
  CEmbodiedEntity& GetEmbodiedEntity() { return *m_body; }
  CWheeledEntity& GetWheeledEntity() { return *m_wheels; }
  CControllableEntity& GetControllableEntity() { return *m_controller; }
  const std::array<Real,2>& GetEncoderDistances() const {return m_encoder_distances;}
  void AccumulateEncoderDistances(Real left,Real right) {m_encoder_distances[0]+=left;m_encoder_distances[1]+=right;}
  Real Mass() const { return m_mass; }
  Real Friction() const { return m_friction; }
  Real MaxForce() const { return m_force; }
  Real MaxTorque() const { return m_torque; }
  static constexpr Real Radius = .054205;
  static constexpr Real Height = .16735;
  static constexpr Real Track = .0795;
  static constexpr Real WheelRadius = .016;
 private:
  std::array<Real,2> m_encoder_distances{};
  void InitTags(TConfigurationNode& tree);
  CTagEquippedEntity* m_tags = nullptr;
  void InitRadios(TConfigurationNode& tree);
  void InitRangeAndBearing(TConfigurationNode& tree);
  CRABEquippedEntity* m_rab = nullptr;
  CSimpleRadioEquippedEntity* m_radios = nullptr;
  CEmbodiedEntity* m_body = nullptr;
  CWheeledEntity* m_wheels = nullptr;
  CControllableEntity* m_controller = nullptr;
  CLEDEquippedEntity* m_leds = nullptr;
  CColor m_diffuser_color=CColor::BLACK;
  CProximitySensorEquippedEntity* m_ir = nullptr;
  Real m_ir_range = .40; // User-specified maximum range; response curve is not calibrated.
  // Initial simulation settings only: not measured hardware properties.
  Real m_mass = .4, m_friction = .7, m_force = 1.5, m_torque = 1.5;
};
}
