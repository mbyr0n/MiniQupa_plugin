#include <argos3/plugins/robots/miniqupa/simulator/miniqupa_entity.h>
#include <argos3/plugins/robots/miniqupa/simulator/miniqupa_tag_pattern.h>
#include <argos3/plugins/simulator/visualizations/qt-opengl/qtopengl_obj_model.h>
#include <argos3/plugins/simulator/visualizations/qt-opengl/qtopengl_widget.h>
#include <argos3/plugins/robots/miniqupa/simulator/miniqupa_model_path.h>
#include <fstream>
#include <sstream>

namespace argos {
namespace {
std::string ReadAsset(const std::string& file) {
 std::ifstream in(MiniQupaModelDirectory()+"/"+file);
 if(!in) THROW_ARGOSEXCEPTION("Cannot read MiniQupa asset " << file);
 std::ostringstream s;std::string line;
 while(std::getline(in,line)) if(line.rfind("mtllib ",0)!=0) s<<line<<"\n";
 return s.str();
}
void DrawTag(const CTagEquippedEntity::SInstance& instance) {
 if(!instance.Tag.IsEnabled())return;
 const auto cells=MiniQupaTagPattern(instance.Tag.GetPayload());
 const Real side=instance.Tag.GetSideLength();CRadians yaw,pitch,roll;instance.OrientationOffset.ToEulerAngles(yaw,pitch,roll);
 glPushAttrib(GL_ENABLE_BIT|GL_CURRENT_BIT|GL_LIGHTING_BIT|GL_POLYGON_BIT);
 glDisable(GL_LIGHTING);glDisable(GL_TEXTURE_2D);glDisable(GL_CULL_FACE);
 glPushMatrix();glTranslated(instance.PositionOffset.GetX(),instance.PositionOffset.GetY(),instance.PositionOffset.GetZ());glRotated(yaw.GetValue()*180/ARGOS_PI,0,0,1);
 glBegin(GL_QUADS);glNormal3d(0,0,1);
 for(size_t r=0;r<10;++r)for(size_t c=0;c<10;++c) {
  const GLfloat color=cells[r*10+c]?1.f:0.f;glColor3f(color,color,color);
  const Real x=(Real(c)-5)*side/8,y=(Real(r)-5)*side/8,cell=side/8;
  glVertex3d(x,y,0);glVertex3d(x+cell,y,0);glVertex3d(x+cell,y+cell,0);glVertex3d(x,y+cell,0);
 }
 glEnd();glPopMatrix();glPopAttrib();
}
}
class CQTOpenGLDrawMiniQupa : public CQTOpenGLOperationDrawNormal {
 public:
  void ApplyTo(CQTOpenGLWidget& visualization,CMiniQupaEntity& entity) {
   static CQTOpenGLObjModel model(ReadAsset("miniqupa.mtl"),ReadAsset("miniqupa.obj"));
   auto& diffuser=model.GetMaterial("miniqupa_diffuser");
   static const auto unlit=diffuser;
   // The model is shared between robots; restore this material for every draw.
   diffuser=unlit;
   const auto& color=entity.GetDiffuserColor();
   if(color!=CColor::BLACK) {
    const std::array<GLfloat,3> rgb={color.GetRed()/255.f,color.GetGreen()/255.f,color.GetBlue()/255.f};
    for(size_t i=0;i<3;++i) {
     diffuser.Ambient[i]=rgb[i]*.3f;
     diffuser.Diffuse[i]=rgb[i]*.5f;
     diffuser.Emission[i]=rgb[i]*.8f;
    }
   }
   visualization.DrawRays(entity.GetControllableEntity());
   visualization.DrawEntity(entity.GetEmbodiedEntity());
   glPushAttrib(GL_LIGHTING_BIT);
   glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
   model.Draw();
   glPopAttrib();
   for(const auto& instance:entity.GetTagEquippedEntity().GetInstances())DrawTag(instance);

  }
};
class CQTOpenGLSelectMiniQupa : public CQTOpenGLOperationDrawSelected {
 public:
  void ApplyTo(CQTOpenGLWidget& visualization,CMiniQupaEntity& entity) {
   visualization.DrawBoundingBox(entity.GetEmbodiedEntity());
  }
};
REGISTER_QTOPENGL_ENTITY_OPERATION(CQTOpenGLOperationDrawNormal,CQTOpenGLDrawMiniQupa,CMiniQupaEntity);
REGISTER_QTOPENGL_ENTITY_OPERATION(CQTOpenGLOperationDrawSelected,CQTOpenGLSelectMiniQupa,CMiniQupaEntity);
}
