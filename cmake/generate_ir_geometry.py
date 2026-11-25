#!/usr/bin/env python3
"""Generate C++ IR poses from the reviewed geometry sheet (metres, robot frame)."""
import json,math,sys
from pathlib import Path
src,dst=map(Path,sys.argv[1:])
sensors=json.loads(src.read_text())['ir_sensors']
if [s['id'] for s in sensors]!=[f'IR{i}' for i in range(1,9)]:raise SystemExit('Expected IR1..IR8 in order')
lines=['// Generated from config/miniqupa_geometry.json; do not edit.','#pragma once','#include <array>','namespace argos {','struct SMiniQupaIRPose { const char* Id; double X,Y,Z,DX,DY,DZ; };','inline constexpr std::array<SMiniQupaIRPose,8> MiniQupaIRPoses = {{']
for s in sensors:
 p=s['position_m'];d=s['direction']
 if len(p)!=3 or len(d)!=3 or not all(math.isfinite(v) for v in p+d) or abs(sum(v*v for v in d)-1)>1e-5:raise SystemExit('Invalid IR pose')
 lines.append(' {"'+s['id']+'",'+','.join(f'{v:.12g}' for v in p+d)+'},')
lines+=['}};','}'];dst.parent.mkdir(parents=True,exist_ok=True);dst.write_text('\n'.join(lines)+'\n')
