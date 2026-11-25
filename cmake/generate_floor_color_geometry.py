#!/usr/bin/env python3
"""Resolve the floor sensor reference once, from the reviewed CAD geometry."""
from pathlib import Path
import json,math,sys

def floor_origin(geometry):
    origin=geometry['floor_rgb']['optical_origin_m']
    if origin is None:
        landmarks={point['id']:point['position_m'] for point in geometry['landmarks']}
        origin=landmarks[geometry['floor_rgb']['simulation_reference_landmark']]
    if len(origin)!=3 or not all(math.isfinite(value) for value in origin) or origin[2]<0:
        raise ValueError('Invalid floor color origin')
    return origin

if __name__=='__main__':
    source,dest=map(Path,sys.argv[1:])
    origin=floor_origin(json.loads(source.read_text()))
    dest.parent.mkdir(parents=True,exist_ok=True)
    dest.write_text('// Generated from config/miniqupa_geometry.json; do not edit.\n#pragma once\n#include <array>\nnamespace argos {\ninline constexpr std::array<double,3> MiniQupaFloorColorOrigin = {'+','.join(f'{v:.12g}' for v in origin)+'};\n}\n')
