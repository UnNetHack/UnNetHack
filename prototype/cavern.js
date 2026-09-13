import * as THREE from 'three';
import {RoundedBoxGeometry} from 'three/addons/geometries/RoundedBoxGeometry.js';
import {cellHash} from './floor.js';

// Underground staging for Live mode: the level is carved out of a rock mass that fills
// every square the hero hasn't uncovered, fading into darkness past the map edge.

const COLS=80,ROWS=21,MARGIN=8,BASE=-.3;
const LOW=.1,MID=.34,HIGH=.6; // rock top next to open ground, one square back, and beyond

function rockTextures(){
 const size=128,mc=document.createElement('canvas'),bc=document.createElement('canvas');mc.width=mc.height=bc.width=bc.height=size;
 const m=mc.getContext('2d'),b=bc.getContext('2d');let n=77;const r=()=>{n=(n*1664525+1013904223)>>>0;return n/4294967296;};
 m.fillStyle='#8a8f8c';m.fillRect(0,0,size,size);b.fillStyle='#808080';b.fillRect(0,0,size,size);
 for(let i=0;i<260;i++){const x=r()*size,y=r()*size,rad=2+r()*12,light=r()<.45;
  m.globalAlpha=.08+r()*.18;m.fillStyle=light?'#b3b6b0':'#4a504d';m.beginPath();m.ellipse(x,y,rad,rad*(.4+r()*.6),r()*3,0,Math.PI*2);m.fill();
  b.globalAlpha=.15+r()*.25;b.fillStyle=light?'#c0c0c0':'#404040';b.beginPath();b.ellipse(x,y,rad,rad*(.4+r()*.6),r()*3,0,Math.PI*2);b.fill();}
 const map=new THREE.CanvasTexture(mc),bump=new THREE.CanvasTexture(bc);map.colorSpace=THREE.SRGBColorSpace;
 return {map,bump};
}

export function createCavern({group,scene,camera,controls}){
 const minX=-MARGIN,minZ=-MARGIN,W=COLS+MARGIN*2,H=ROWS+MARGIN*2,count=W*H;
 const {map,bump}=rockTextures();
 const mesh=new THREE.InstancedMesh(new RoundedBoxGeometry(1,1,1,2,.09),new THREE.MeshStandardMaterial({map,bumpMap:bump,bumpScale:3,roughness:1}),count);
 mesh.receiveShadow=true;mesh.frustumCulled=false;group.add(mesh);
 const current=new Float32Array(count).fill(BASE),target=new Float32Array(count).fill(BASE),color=new THREE.Color();
 for(let i=0;i<count;i++){const h=cellHash(i,1,9);mesh.setColorAt(i,color.setRGB(.12,.13,.135).multiplyScalar(.7+(h%100)/200));}
 let originX=0,originZ=0,animating=false;
 const matrix=new THREE.Matrix4(),position=new THREE.Vector3(),quaternion=new THREE.Quaternion(),scale=new THREE.Vector3(),euler=new THREE.Euler();

 function compose(i){
  const x=i%W+minX,z=Math.floor(i/W)+minZ,h=cellHash(x,z,8),height=current[i]-BASE;
  if(height<.005){matrix.makeScale(0,0,0);mesh.setMatrixAt(i,matrix);return;}
  // irregular boulders rather than a grid: each block wanders, leans and swells a little
  const rough=Math.min(1,height/(HIGH-BASE));
  position.set(x-originX+((h&15)/15-.5)*.22*rough,BASE+height/2,z-originZ+((h>>>4&15)/15-.5)*.22*rough);
  quaternion.setFromEuler(euler.set(((h>>>12&15)/15-.5)*.14*rough,((h>>>8&15)/15-.5)*.9,((h>>>16&15)/15-.5)*.14*rough));
  const swell=1.02+((h>>>20&15)/15)*.22*rough;scale.set(swell,height,swell*(.9+((h>>>24&15)/15)*.2));
  mesh.setMatrixAt(i,matrix.compose(position,quaternion,scale));
 }

 // Called after each engine frame. Squares the hero knows are open; the rock around
 // open ground stays low so it never hides a corridor from the camera.
 function rebuild(tiles,origin,snap){
  originX=origin.x;originZ=origin.z;
  const dist=new Uint8Array(count).fill(255),queue=[];
  for(const [id,tile] of tiles){if(tile.userData.type==='wall')continue;const [x,z]=id.split(',').map(Number),i=(z-minZ)*W+(x-minX);if(i>=0&&i<count){dist[i]=0;queue.push(i);}}
  for(let q=0;q<queue.length;q++){const i=queue[q],d=dist[i];if(d>=2)continue;const x=i%W,z=Math.floor(i/W);
   for(let dz=-1;dz<=1;dz++)for(let dx=-1;dx<=1;dx++){const nx=x+dx,nz=z+dz;if(nx<0||nz<0||nx>=W||nz>=H)continue;const j=nz*W+nx;if(dist[j]>d+1){dist[j]=d+1;queue.push(j);}}}
  for(let i=0;i<count;i++){
   const x=i%W+minX,z=Math.floor(i/W)+minZ;
   const jitter=((cellHash(x,z,7)%100)/100-.5)*.22;
   const outside=x<1||x>=COLS||z<0||z>=ROWS;
   target[i]=tiles.has(`${x},${z}`)?BASE:dist[i]<=1?LOW+jitter*.3:dist[i]===2?MID+jitter*.6:HIGH+jitter*(outside?2:1.2);
   if(snap)current[i]=target[i];
  }
  animating=true;
 }

 // Distance fog so the rock sinks into darkness instead of ending at a hard edge.
 const fog=new THREE.Fog('#05080b',10,30),background=new THREE.Color('#05080b');let saved=null;
 function setActive(active){
  if(active&&!saved){saved={fog:scene.fog,background:scene.background};scene.fog=fog;scene.background=background;}
  else if(!active&&saved){scene.fog=saved.fog;scene.background=saved.background;saved=null;}
 }

 // Dust hanging in the air around the hero, catching the lantern and torchlight.
 const MOTES=180,moteGeo=new THREE.BufferGeometry(),motePos=new Float32Array(MOTES*3),moteSeed=new Float32Array(MOTES);
 for(let i=0;i<MOTES;i++){motePos[i*3]=(Math.random()-.5)*12;motePos[i*3+1]=Math.random()*2.6;motePos[i*3+2]=(Math.random()-.5)*12;moteSeed[i]=Math.random()*100;}
 moteGeo.setAttribute('position',new THREE.BufferAttribute(motePos,3));
 const motes=new THREE.Points(moteGeo,new THREE.PointsMaterial({color:'#ffdcaa',size:.04,transparent:true,opacity:.55,depthWrite:false,blending:THREE.AdditiveBlending}));
 motes.frustumCulled=false;group.add(motes);

 function update(t,dt,hero){
  const d=camera.position.distanceTo(controls.target);fog.near=d*.9;fog.far=d*2.1;
  if(animating){
   const ease=1-Math.exp(-dt*6);animating=false;
   for(let i=0;i<count;i++){const delta=target[i]-current[i];if(delta){current[i]=Math.abs(delta)<.003?target[i]:current[i]+delta*ease;if(current[i]!==target[i])animating=true;}compose(i);}
   mesh.instanceMatrix.needsUpdate=true;
  }
  motes.position.set(hero.x,0,hero.z);
  for(let i=0;i<MOTES;i++){const s=moteSeed[i];let y=motePos[i*3+1]+dt*(.04+(s%1)*.05);if(y>2.6)y-=2.6;motePos[i*3+1]=y;motePos[i*3]+=Math.sin(t*.3+s)*dt*.06;motePos[i*3+2]+=Math.cos(t*.27+s*1.3)*dt*.06;}
  moteGeo.attributes.position.needsUpdate=true;
 }
 return {rebuild,update,setActive};
}
