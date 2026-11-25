// Visibility follows the ARGoS LED-medium camera pattern. Grouping uses only
// observed color and local azimuth; no source ID is used or exposed.
#include <argos3/plugins/robots/miniqupa/control_interface/ci_miniqupa_camera_sensor.h>
#include <argos3/core/simulator/sensor.h>
#include <argos3/core/simulator/simulator.h>
#include <argos3/core/simulator/space/space.h>
#include <argos3/core/simulator/entity/composable_entity.h>
#include <argos3/core/simulator/entity/embodied_entity.h>
#include <argos3/core/simulator/entity/controllable_entity.h>
#include <argos3/plugins/simulator/media/led_medium.h>
#include <algorithm>
#include <cmath>
#include <map>
#include <numeric>
namespace argos {
class CMiniQupaCameraDefaultSensor : public CSimulatedSensor, public CCI_MiniQupaCameraSensor {
 struct SSample { CColor Color; Real Angle; Real HalfWidth; };
 class CCollect : public CPositionalIndex<CLEDEntity>::COperation {
  public:
   explicit CCollect(CMiniQupaCameraDefaultSensor& camera):m_camera(camera) {}
   bool operator()(CLEDEntity& led) override {m_camera.Consider(led);return true;}
  private: CMiniQupaCameraDefaultSensor& m_camera;
 };
 public:
  void SetRobot(CComposableEntity& entity) override {
   m_robot=&entity;m_body=&entity.GetComponent<CEmbodiedEntity>("body");m_controller=&entity.GetComponent<CControllableEntity>("controller");
  }
  void Init(TConfigurationNode& tree) override {
   std::string medium="leds";GetNodeAttributeOrDefault(tree,"medium",medium,medium);
   m_medium=&CSimulator::GetInstance().GetMedium<CLEDMedium>(medium);
   GetNodeAttributeOrDefault(tree,"range",m_range,m_range);
   GetNodeAttributeOrDefault(tree,"height",m_height,m_height);
   GetNodeAttributeOrDefault(tree,"vertical_half_angle_deg",m_vertical_half_deg,m_vertical_half_deg);
   GetNodeAttributeOrDefault(tree,"merge_gap_deg",m_merge_deg,m_merge_deg);
   GetNodeAttributeOrDefault(tree,"show_rays",m_show_rays,m_show_rays);
   GetNodeAttributeOrDefault(tree,"sample_radius",m_patch_radius,m_patch_radius);
   if(!std::isfinite(m_range)||m_range<=0||!std::isfinite(m_height)||m_height<=0||!std::isfinite(m_vertical_half_deg)||m_vertical_half_deg<=0||m_vertical_half_deg>90||!std::isfinite(m_merge_deg)||m_merge_deg<0||m_merge_deg>180||!std::isfinite(m_patch_radius)||m_patch_radius<0)
    THROW_ARGOSEXCEPTION("Invalid MiniQupa panoramic camera parameters");
   m_merge=m_merge_deg*ARGOS_PI/180;m_vertical_half=m_vertical_half_deg*ARGOS_PI/180;
   Reset();Enable();
  }
  void Update() override {
   if(IsDisabled()) return;
   m_samples.clear();m_blobs.clear();m_raw_count=0;
   const auto& anchor=m_body->GetOriginAnchor();
   m_position=CVector3(0,0,m_height);m_position.Rotate(anchor.Orientation);m_position+=anchor.Position;
   m_inverse=anchor.Orientation.Inverse();
   CCollect collect(*this);
   // range limits horizontal distance. Search the arena's full height so
   // high sources inside the vertical field of view reach Consider().
   const auto& space=CSimulator::GetInstance().GetSpace();
   CVector3 center=m_position;center.SetZ(space.GetArenaCenter().GetZ());
   m_medium->GetIndex().ForEntitiesInBoxRange(center,CVector3(m_range,m_range,space.GetArenaSize().GetZ()/2),collect);
   m_raw_count=m_samples.size();
   Group();
  }
  void Reset() override {m_samples.clear();m_blobs.clear();m_raw_count=0;}
 private:
  void Consider(CLEDEntity& led) {
   if(led.GetColor()==CColor::BLACK) return;
   CEntity* root=&led;while(root->HasParent()) root=&root->GetParent();
   if(root==m_robot) return; // Only self exclusion; never used for grouping.
   CVector3 local=led.GetPosition()-m_position;local.Rotate(m_inverse);
   const Real horizontal=std::hypot(local.GetX(),local.GetY());
   if(horizontal>m_range || horizontal<1e-9 || Abs(std::atan2(local.GetZ(),horizontal))>m_vertical_half) return;
   const CRay3 ray(m_position,led.GetPosition());SEmbodiedEntityIntersectionItem intersection;
   const bool blocked=GetClosestEmbodiedEntityIntersectedByRay(intersection,ray,*m_body);
   if(m_show_rays) {m_controller->AddCheckedRay(blocked,ray);if(blocked)m_controller->AddIntersectionPoint(ray,intersection.TOnRay);}
   if(!blocked) m_samples.push_back({led.GetColor(),std::atan2(local.GetY(),local.GetX()),std::atan2(m_patch_radius,horizontal)});
  }
  void Group() {
   const size_t n=m_samples.size();if(!n)return;
   std::sort(m_samples.begin(),m_samples.end(),[](const auto& a,const auto& b){return a.Angle<b.Angle;});
   std::vector<size_t> parent(n);std::iota(parent.begin(),parent.end(),0);
   auto find=[&](size_t i){while(parent[i]!=i){parent[i]=parent[parent[i]];i=parent[i];}return i;};
   for(size_t i=0;i<n;++i) {
    const size_t j=(i+1)%n;
    Real gap=m_samples[j].Angle-m_samples[i].Angle;if(j==0)gap+=2*ARGOS_PI;
    if(m_samples[i].Color==m_samples[j].Color && gap<=m_samples[i].HalfWidth+m_samples[j].HalfWidth+m_merge+1e-12) parent[find(j)]=find(i);
   }
   std::map<size_t,std::vector<size_t>> groups;
   for(size_t i=0;i<n;++i)groups[find(i)].push_back(i);
   for(const auto& pair:groups) {
    const auto& indices=pair.second;Real sx=0,sy=0,max_gap=0;
    for(size_t k=0;k<indices.size();++k) {
     Real angle=m_samples[indices[k]].Angle;sx+=std::cos(angle);sy+=std::sin(angle);
     Real next=m_samples[indices[(k+1)%indices.size()]].Angle;
     Real gap=next-angle;if(k+1==indices.size())gap+=2*ARGOS_PI;max_gap=std::max(max_gap,gap-m_samples[indices[k]].HalfWidth-m_samples[indices[(k+1)%indices.size()]].HalfWidth);
    }
    m_blobs.push_back({m_samples[indices[0]].Color,CRadians(std::atan2(sy,sx)),CRadians(2*ARGOS_PI-max_gap),indices.size()});
   }
   std::sort(m_blobs.begin(),m_blobs.end(),[](const auto& a,const auto& b){return a.Angle.GetValue()<b.Angle.GetValue();});
  }
  CComposableEntity* m_robot=nullptr;CEmbodiedEntity* m_body=nullptr;CControllableEntity* m_controller=nullptr;CLEDMedium* m_medium=nullptr;
  Real m_range=.8,m_height=.15,m_vertical_half_deg=45,m_merge_deg=1,m_vertical_half=0,m_merge=0,m_patch_radius=ARGOS_PI*.0544/16;
  bool m_show_rays=false;CVector3 m_position;CQuaternion m_inverse;std::vector<SSample> m_samples;
};
REGISTER_SENSOR(CMiniQupaCameraDefaultSensor,"miniqupa_camera","default","MiniQupa workspace","0.1","Ideal panoramic diffuser blob camera.","360 degree azimuth visibility with configurable range, virtual height, vertical half angle and occlusion. Projects each sample as a small diffuser patch and groups touching equal-color angular intervals with a configurable gap, including wraparound. No robot IDs or ground-truth distances returned. Not a calibrated mirror projection or an RGB image camera.","Usable");
}
