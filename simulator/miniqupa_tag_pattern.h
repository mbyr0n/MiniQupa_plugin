#pragma once
#include <array>
#include <charconv>
#include <cstdint>
#include <string>
namespace argos {
inline bool MiniQupaParseTagId(const std::string& text,std::uint32_t& id) {
 const auto result=std::from_chars(text.data(),text.data()+text.size(),id);
 return result.ec==std::errc{} && result.ptr==text.data()+text.size() && !text.empty();
}
/** Visual-only marker. Native ARGoS detection uses CTagEntity, not these cells. */
inline std::array<unsigned char,100> MiniQupaTagPattern(const std::string& payload) {
 std::uint32_t code=0;
 if(!MiniQupaParseTagId(payload,code)) {
  code=2166136261u;for(unsigned char ch:payload)code=(code^ch)*16777619u;
 }
 code=code*0x9e3779b1u^0xa5a5a5a5u;
 std::array<unsigned char,100> cells;cells.fill(1);
 for(size_t r=1;r<9;++r)for(size_t c=1;c<9;++c)cells[r*10+c]=0;
 unsigned bit=0;
 for(size_t r=2;r<8;++r)for(size_t c=2;c<8;++c) {
  const bool corner=(r==2||r==7)&&(c==2||c==7);
  cells[r*10+c]=corner?(r==2&&c==2):((code>>bit++)&1u);
 }
 return cells;
}
}
