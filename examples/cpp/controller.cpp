#include <argos3/core/control_interface/ci_controller.h>
#include <argos3/core/utility/logging/argos_log.h>
#include <argos3/plugins/robots/generic/control_interface/ci_differential_steering_actuator.h>
#include <argos3/plugins/robots/generic/control_interface/ci_simple_radios_actuator.h>
#include <argos3/plugins/robots/generic/control_interface/ci_simple_radios_sensor.h>
#include <argos3/plugins/robots/generic/control_interface/ci_range_and_bearing_actuator.h>
#include <argos3/plugins/robots/generic/control_interface/ci_range_and_bearing_sensor.h>
#include <argos3/plugins/robots/miniqupa/control_interface/ci_miniqupa_ir_sensor.h>
#include <argos3/plugins/robots/miniqupa/control_interface/ci_miniqupa_encoder_sensor.h>
#include <argos3/plugins/robots/miniqupa/control_interface/ci_miniqupa_floor_color_sensor.h>
#include <argos3/plugins/robots/miniqupa/control_interface/ci_miniqupa_camera_sensor.h>
#include <argos3/plugins/robots/miniqupa/control_interface/ci_miniqupa_diffuser_actuator.h>
#include <cmath>
using namespace argos;

// Controlador didáctico: los dispositivos son propiedad de ARGoS.
class CMiniQupaManualController : public CCI_Controller {
 public:
  void Init(TConfigurationNode&) override {
    wheels = GetActuator<CCI_DifferentialSteeringActuator>("differential_steering");
    diffuser = GetActuator<CCI_MiniQupaDiffuserActuator>("miniqupa_diffuser");
    ir = GetSensor<CCI_MiniQupaIRSensor>("miniqupa_ir");
    encoder = GetSensor<CCI_MiniQupaEncoderSensor>("miniqupa_encoder");
    floor = GetSensor<CCI_MiniQupaFloorColorSensor>("miniqupa_floor_color");
    camera = GetSensor<CCI_MiniQupaCameraSensor>("miniqupa_camera");
    radio_tx = GetActuator<CCI_SimpleRadiosActuator>("simple_radios");
    radio_rx = GetSensor<CCI_SimpleRadiosSensor>("simple_radios");
    rab_tx = GetActuator<CCI_RangeAndBearingActuator>("range_and_bearing");
    rab_rx = GetSensor<CCI_RangeAndBearingSensor>("range_and_bearing");
    Reset();
  }
  void Reset() override {
    tick = 0;
    distance = 0;
    diffuser->TurnOff();
    rab_tx->ClearData();
  }
  void ControlStep() override {
    ++tick;
    wheels->SetLinearVelocity(tick <= 40 ? 2 : 0, tick <= 40 ? 2 : 0);
    diffuser->SetColor(CColor(0, 0, 255));
    const auto& e = encoder->GetReading();
    distance += (e.CoveredDistanceLeftWheel + e.CoveredDistanceRightWheel) / 2;
    if(ir->GetReadings().size() != 8) THROW_ARGOSEXCEPTION("Expected eight IR readings");
    for(const auto& reading : ir->GetReadings()) {
      if(reading.Value < 0 || reading.Value > 1) THROW_ARGOSEXCEPTION("Invalid proximity");
    }
    if(tick > 1 && (!floor->GetReading().Valid || floor->GetReading().Color != CColor::WHITE))
      THROW_ARGOSEXCEPTION("Expected valid white floor");
    if(!camera->GetBlobs().empty() || camera->GetRawSampleCount() != 0)
      THROW_ARGOSEXCEPTION("Single robot should not see itself");
    SendBinaryRadio();
    for(const auto& interface : radio_rx->GetInterfaces()) {
      if(!interface.Messages.empty()) THROW_ARGOSEXCEPTION("Unexpected radio peer");
    }
    CByteArray data(rab_tx->GetSize());
    data.Zero(); data[0] = 42;
    rab_tx->SetData(data);
    rab_tx->SetData(1, 9); // Segundo byte, índice C++ 1.
    if(!rab_rx->GetReadings().empty()) THROW_ARGOSEXCEPTION("Unexpected RAB peer");
    if(tick == 100) {
      if(distance < .03 || distance > .05) THROW_ARGOSEXCEPTION("Unexpected encoder travel");
      LOG << "MINIQUPA_CPP_OK" << std::endl;
    }
  }
 private:
  void SendBinaryRadio() {
    for(auto& interface : radio_tx->GetInterfaces()) {
      if(interface.Id == "espnow") {
        CByteArray bytes;
        bytes << UInt8(42) << UInt8(7);
        interface.Messages.push_back(bytes);
      }
    }
  }
  CCI_DifferentialSteeringActuator* wheels = nullptr;
  CCI_MiniQupaDiffuserActuator* diffuser = nullptr;
  CCI_MiniQupaIRSensor* ir = nullptr;
  CCI_MiniQupaEncoderSensor* encoder = nullptr;
  CCI_MiniQupaFloorColorSensor* floor = nullptr;
  CCI_MiniQupaCameraSensor* camera = nullptr;
  CCI_SimpleRadiosActuator* radio_tx = nullptr;
  CCI_SimpleRadiosSensor* radio_rx = nullptr;
  CCI_RangeAndBearingActuator* rab_tx = nullptr;
  CCI_RangeAndBearingSensor* rab_rx = nullptr;
  unsigned tick = 0;
  Real distance = 0;
};
REGISTER_CONTROLLER(CMiniQupaManualController, "miniqupa_manual_controller");
