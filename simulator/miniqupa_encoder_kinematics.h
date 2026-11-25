#pragma once
#include <array>
#include <cmath>
namespace argos {
/** Equivalent no-slip wheel travel from a small actual physics pose increment.
 * Midpoint projection is integrated at each physics substep, not from commands.
 */
inline std::array<double,2> MiniQupaRollingIncrement(double dx,double dy,double yaw_before,double yaw_after,double track) {
 const double middle=(yaw_before+yaw_after)*.5;
 const double forward=dx*std::cos(middle)+dy*std::sin(middle);
 const double turn=(yaw_after-yaw_before)*track*.5;
 return {forward-turn,forward+turn};
}
}
