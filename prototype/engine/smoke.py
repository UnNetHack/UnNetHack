#!/usr/bin/env python3
"""Real-engine smoke test with its own character, entirely within .engine/runtime."""
import json, os, subprocess, time, selectors
from pathlib import Path
root=Path(__file__).resolve().parents[1]/'.engine'
m=json.loads((root/'manifest.json').read_text())
assert Path(m['prefix']).resolve().is_relative_to(root.resolve())
env={**os.environ,'HOME':m['home'],'NETHACKOPTIONS':'windowtype:bridge,name:BridgeSmoke,role:Valkyrie,race:human,gender:female,align:lawful,pettype:cat,!news,!autopickup'}
p=subprocess.Popen([m['binary'],'-d',m['cwd'],'-u','BridgeSmoke','-p','Valkyrie','-r','human'],cwd=m['cwd'],env=env,stdin=subprocess.PIPE,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
sel=selectors.DefaultSelector();sel.register(p.stdout,selectors.EVENT_READ);sel.register(p.stderr,selectors.EVENT_READ)
buf=b'';stage=0;frame=None;start_turn=None;inventory=False;target=None;waited_turn=None;deadline=time.monotonic()+30;messages=[]
try:
 while time.monotonic()<deadline and p.poll() is None:
  for key,_ in sel.select(.3):
   b=os.read(key.fileobj.fileno(),65536)
   if not b:sel.unregister(key.fileobj);continue
   if key.fileobj is p.stderr:messages.append(b.decode(errors='replace'));continue
   buf+=b
   while b'\n' in buf:
    line,buf=buf.split(b'\n',1)
    if not line:continue
    try:v=json.loads(line)
    except Exception:raise AssertionError('Non-JSON engine output: '+line[:250].decode(errors='replace'))
    if v['type']=='frame':frame=v
    elif v['type']=='message':messages.append(v['text'])
    elif v['type'] in ('text','menu'):
     if stage==2:inventory=True
    elif v['type']=='request':
     kind=v['kind'];reply='13'
     if kind=='command':
      assert frame and frame['player']['hp']>0 and frame['cells']
      if stage==0:
       start_turn=frame['turn'];reply='46'
       for dx,dz,keycode in [(1,0,108),(-1,0,104),(0,1,106),(0,-1,107)]:
        x,z=frame['player']['x']+dx,frame['player']['z']+dz
        if any(c['x']==x and c['z']==z and c['kind']=='terrain' and c['terrain']=='floor' for c in frame['cells']):
         target=(x,z);reply=str(keycode);break
       stage=1
      elif stage==1:
       assert frame['turn']>start_turn
       if target:assert (frame['player']['x'],frame['player']['z'])==target
       waited_turn=frame['turn'];reply='105';stage=2
      elif stage==2:
       assert inventory and frame['turn']==waited_turn
       hero_cell=next(c for c in frame['cells'] if c['x']==frame['player']['x'] and c['z']==frame['player']['z'])
       assert hero_cell['terrain']!='unknown',hero_cell
       reply='83';stage=3
      else:raise AssertionError('Save did not exit')
     elif kind=='key':reply='121' if stage==3 else '110'
     elif kind=='menu':reply=''
     elif kind=='line':reply='\x1b'
     p.stdin.write((reply+'\n').encode());p.stdin.flush()
 if p.poll() is None:raise AssertionError('Engine smoke test timed out')
 assert stage==3 and inventory,(stage,messages[-10:])
 saves=list((Path(m['prefix'])/'var/unnethack/saves').glob('*BridgeSmoke*'))
 assert saves,'Save file missing'
 print(json.dumps({'turn_before':start_turn,'turn_after':frame['turn'],'observed_cells':len(frame['cells']),'inventory':inventory,'movement_target':target,'save_files':[s.name for s in saves],'exit':p.returncode}))
finally:
 if p.poll() is None:
  p.stdin.close()
  try:p.wait(timeout=5)
  except subprocess.TimeoutExpired:p.terminate();p.wait(timeout=5)
