#!/usr/bin/env python3
"""Build default tag mounting parameters from the reviewed CAD metadata."""
from pathlib import Path
import json,math,sys

def tag_defaults(geometry):
    config=geometry['apriltag'];position=config['simulation_position_m'];side=config['simulation_side_length_m']
    if len(position)!=3 or not all(math.isfinite(v) for v in position) or not math.isfinite(side) or side<=0:
        raise ValueError('Invalid tag geometry')
    return position,side

if __name__=='__main__':
    source,target=map(Path,sys.argv[1:]);position,side=tag_defaults(json.loads(source.read_text()))
    target.parent.mkdir(parents=True,exist_ok=True)
    target.write_text('// Generated from config/miniqupa_geometry.json; do not edit.\n#pragma once\n#include <array>\nnamespace argos {\ninline constexpr std::array<double,3> MiniQupaTagPosition = {'+','.join(f'{v:.12g}' for v in position)+'};\ninline constexpr double MiniQupaTagSide = '+str(side)+';\n}\n')
