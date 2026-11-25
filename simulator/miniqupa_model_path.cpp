#include <argos3/plugins/robots/miniqupa/simulator/miniqupa_model_path.h>
#include <argos3/core/utility/configuration/argos_exception.h>
#include <cstdlib>
#include <filesystem>
#include <dlfcn.h>
namespace argos {
namespace {
std::string CheckedDirectory(const std::filesystem::path& path) {
 if(!std::filesystem::is_regular_file(path/"miniqupa.obj") || !std::filesystem::is_regular_file(path/"miniqupa.mtl"))
  THROW_ARGOSEXCEPTION("Missing MiniQupa model assets in "<<path.string());
 return path.string();
}
}
std::string MiniQupaModelDirectory() {
 if(const char* override_path=std::getenv("ARGOS_MINIQUPA_MODEL_DIR"))return CheckedDirectory(override_path);
 static const int library_anchor=0;Dl_info library{};
 if(dladdr(&library_anchor,&library)!=0) {
  const auto directory=std::filesystem::path(library.dli_fname).parent_path();
  if(directory.filename()=="argos3" && directory.parent_path().filename()=="lib")
   return CheckedDirectory(directory.parent_path().parent_path()/"share/argos3/robots/miniqupa/models");
 }
 return CheckedDirectory(MINIQUPA_SOURCE_MODEL_DIR);
}
}
