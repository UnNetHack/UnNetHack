import {installLive} from './live.js';
import * as THREE from 'three';
import { OrbitControls } from 'three/addons/controls/OrbitControls.js';
import { EffectComposer } from 'three/addons/postprocessing/EffectComposer.js';
import { RenderPass } from 'three/addons/postprocessing/RenderPass.js';
import { UnrealBloomPass } from 'three/addons/postprocessing/UnrealBloomPass.js';
import { SSAOPass } from 'three/addons/postprocessing/SSAOPass.js';
import { OutputPass } from 'three/addons/postprocessing/OutputPass.js';
import { RoundedBoxGeometry } from 'three/addons/geometries/RoundedBoxGeometry.js';
import {createState,act,fountain} from './simulation.js';
import {createCreature} from './creatures.js';
import {stageCreature} from './readability.js';
import {createAltar} from './altar.js';
import {createFire} from './fire.js';
import {createHeldWeapon} from './equipment.js';

const scene=new THREE.Scene();scene.background=new THREE.Color('#142333');scene.fog=new THREE.FogExp2('#1b3040',.032);
const renderer=new THREE.WebGLRenderer({antialias:true,alpha:true});renderer.setClearColor(0x000000,0);renderer.setPixelRatio(Math.min(devicePixelRatio,2));renderer.setSize(innerWidth,innerHeight);renderer.shadowMap.enabled=true;renderer.shadowMap.type=THREE.PCFSoftShadowMap;renderer.toneMapping=THREE.ACESFilmicToneMapping;renderer.toneMappingExposure=1.2;document.querySelector('#scene').appendChild(renderer.domElement);
const camera=new THREE.PerspectiveCamera(36,innerWidth/innerHeight,.1,100);camera.position.set(11,13,16);
const controls=new OrbitControls(camera,renderer.domElement);controls.target.set(0,.1,0);controls.enableDamping=true;controls.minDistance=10;controls.maxDistance=27;controls.minPolarAngle=.3;controls.maxPolarAngle=1.22;controls.enablePan=false;
const composer=new EffectComposer(renderer);composer.addPass(new RenderPass(scene,camera));const ambientOcclusion=new SSAOPass(scene,camera,innerWidth,innerHeight);ambientOcclusion.kernelRadius=12;ambientOcclusion.minDistance=.002;ambientOcclusion.maxDistance=.14;composer.addPass(ambientOcclusion);const bloom=new UnrealBloomPass(new THREE.Vector2(innerWidth,innerHeight),.25,.48,1.25);composer.addPass(bloom);composer.addPass(new OutputPass());
scene.add(new THREE.HemisphereLight(0x92bfc4,0x333025,1.5));const moon=new THREE.DirectionalLight(0x9fb9ca,2.3);moon.position.set(-4,10,3);moon.castShadow=true;moon.shadow.mapSize.set(2048,2048);Object.assign(moon.shadow.camera,{left:-9,right:9,top:9,bottom:-9,near:.5,far:30});moon.shadow.bias=-.0003;moon.shadow.normalBias=.03;scene.add(moon);
const mat=(color,extra={})=>new THREE.MeshStandardMaterial({color,roughness:.85,...extra});
function surfaceTexture(base, accent, repeat=1){const canvas=document.createElement('canvas');canvas.width=canvas.height=256;const ctx=canvas.getContext('2d');ctx.fillStyle=base;ctx.fillRect(0,0,256,256);let n=19;const noise=()=>{n=(n*1664525+1013904223)>>>0;return n/4294967296;};for(let i=0;i<760;i++){const x=noise()*256,y=noise()*256,r=1+noise()*7;ctx.globalAlpha=.05+noise()*.15;ctx.fillStyle=accent;ctx.beginPath();ctx.ellipse(x,y,r,r*(.25+noise()*.8),noise()*Math.PI,0,Math.PI*2);ctx.fill();}ctx.globalAlpha=.3;for(let i=0;i<30;i++){ctx.strokeStyle=accent;ctx.lineWidth=.3+noise();ctx.beginPath();let x=noise()*256,y=noise()*256;ctx.moveTo(x,y);for(let j=0;j<4;j++){x+=noise()*34-17;y+=noise()*34-17;ctx.lineTo(x,y);}ctx.stroke();}ctx.globalAlpha=1;const map=new THREE.CanvasTexture(canvas);map.colorSpace=THREE.SRGBColorSpace;map.wrapS=map.wrapT=THREE.RepeatWrapping;map.repeat.set(repeat,repeat);const bumpCanvas=document.createElement('canvas');bumpCanvas.width=bumpCanvas.height=256;const bumpCtx=bumpCanvas.getContext('2d');bumpCtx.fillStyle='#777';bumpCtx.fillRect(0,0,256,256);for(let i=0;i<360;i++){bumpCtx.globalAlpha=.12+noise()*.42;bumpCtx.fillStyle=noise()>.5?'#bbb':'#333';bumpCtx.beginPath();bumpCtx.ellipse(noise()*256,noise()*256,1+noise()*8,1+noise()*4,noise()*Math.PI,0,Math.PI*2);bumpCtx.fill();}const bump=new THREE.CanvasTexture(bumpCanvas);bump.wrapS=bump.wrapT=THREE.RepeatWrapping;bump.repeat.set(repeat,repeat);return {map,bump};}
const stoneSurface=surfaceTexture('#56625f','#20333a',.7),floorSurface=surfaceTexture('#68736c','#2c4140',1.15),metalSurface=surfaceTexture('#9db6bd','#42565a',1.3);
const stone=mat('#68746e',{map:stoneSurface.map,bumpMap:stoneSurface.bump,bumpScale:.12,roughness:.88}),darkStone=mat('#303b3b',{map:stoneSurface.map,bumpMap:stoneSurface.bump,bumpScale:.16,roughness:.92}),trim=mat('#85877a',{map:stoneSurface.map,bumpMap:stoneSurface.bump,bumpScale:.09,roughness:.7}),gold=mat('#b49355',{metalness:.75,roughness:.3}),steel=mat('#a6bec3',{map:metalSurface.map,bumpMap:metalSurface.bump,bumpScale:.07,metalness:.82,roughness:.25}),cloth=mat('#315b59',{map:floorSurface.map,bumpMap:floorSurface.bump,bumpScale:.06,roughness:.94});
let seed=42;function rand(){seed=(seed*1664525+1013904223)>>>0;return seed/4294967296;}
function mesh(geo,material,parent=scene,x=0,y=0,z=0){const m=new THREE.Mesh(geo,material);m.position.set(x,y,z);m.castShadow=true;m.receiveShadow=true;parent.add(m);return m;}
function box(w,h,d,m,p=scene,x=0,y=0,z=0){return mesh(new THREE.BoxGeometry(w,h,d),m,p,x,y,z);}
function roundedBox(w,h,d,m,p=scene,x=0,y=0,z=0,r=.04){return mesh(new RoundedBoxGeometry(w,h,d,3,r),m,p,x,y,z);}
function sphere(r,m,p,x=0,y=0,z=0,sx=1,sy=1,sz=1){const o=mesh(new THREE.SphereGeometry(r,16,12),m,p,x,y,z);o.scale.set(sx,sy,sz);return o;}
function cylinder(r1,r2,h,m,p=scene,x=0,y=0,z=0,n=24){return mesh(new THREE.CylinderGeometry(r1,r2,h,n),m,p,x,y,z);}
function torus(r,t,m,p=scene,x=0,y=0,z=0){const o=mesh(new THREE.TorusGeometry(r,t,8,48),m,p,x,y,z);o.rotation.x=Math.PI/2;return o;}
box(10,.6,8,darkStone,scene,0,-.42,0);box(10.2,.15,8.2,trim,scene,0,-.76,0);box(10,.3,8,darkStone,scene,0,-.96,0);
for(let x=-4;x<=4;x++)for(let z=-3;z<=3;z++){
 const c=new THREE.Color('#748078').multiplyScalar(.72+rand()*.4);const tile=roundedBox(.974,.18,.974,mat(c,{map:floorSurface.map,bumpMap:floorSurface.bump,bumpScale:.13,roughness:.9}),scene,x,-.085+rand()*.013,z,.035);tile.rotation.y=(rand()-.5)*.016;
 if(rand()<.25){const crack=box(.003,.003,.35,mat('#303b35'),scene,x+(rand()-.5)*.6,.015,z);crack.rotation.y=rand()*3;}
}
// Open near walls retain a readable view into the room.
for(let row=0;row<5;row++)for(let i=0;i<10;i++){
 let x=-4.5+i+(row%2)*.25;if(x>4.5)continue;
 if(Math.abs(x)<.9 && row<4)continue;
 roundedBox(.95,.43,.42,row%2?stone:darkStone,scene,x,.23+row*.46,-3.85,.045);
}
for(let row=0;row<3;row++)for(let i=0;i<8;i++)roundedBox(.42,.43,.95,row%2?stone:darkStone,scene,-4.85,.23+row*.46,-3.5+i,.045);
box(9.8,.16,.6,trim,scene,0,2.45,-3.85);
// Recessed doorway and brass keystone.
box(1.65,2,.08,mat('#091213'),scene,0,1,-4.08);
for(const x of [-.95,.95]){box(.27,2.2,.65,trim,scene,x,1.1,-3.85);box(.4,.18,.8,gold,scene,x,.1,-3.85);}
box(2.25,.25,.65,trim,scene,0,2.18,-3.85);box(.28,.35,.73,gold,scene,0,2.2,-3.82);
for(const [x,z] of [[-4.55,-3.5],[4.55,-3.5],[-4.55,3.5],[4.55,3.5]]){
 const h=z>0?.7:2.8;box(.72,.18,.72,trim,scene,x,.08,z);box(.48,h,.48,stone,scene,x,h/2,z);box(.7,.2,.7,trim,scene,x,h,z);box(.53,.09,.53,gold,scene,x,h-.24,z);
}
// Rubble and moss, deterministically placed along room edges.
const moss=mat('#465b39');for(let i=0;i<60;i++){const x=-4.3+rand()*8.6,z=rand()<.5?-3.35:3.35;const o=mesh(new THREE.DodecahedronGeometry(.04+rand()*.11),i%3?stone:moss,scene,x,.05,z);o.scale.y=.4;o.rotation.set(rand(),rand(),rand());}
function haze(color,opacity,x,y,z,sx,sy){const c=document.createElement('canvas');c.width=c.height=256;const ctx=c.getContext('2d');const gradient=ctx.createRadialGradient(128,128,0,128,128,128);gradient.addColorStop(0,color);gradient.addColorStop(.48,`${color}88`);gradient.addColorStop(1,`${color}00`);ctx.fillStyle=gradient;ctx.fillRect(0,0,256,256);const texture=new THREE.CanvasTexture(c);texture.colorSpace=THREE.SRGBColorSpace;const material=new THREE.SpriteMaterial({map:texture,transparent:true,opacity,depthWrite:false,blending:THREE.AdditiveBlending});const sprite=new THREE.Sprite(material);sprite.position.set(x,y,z);sprite.scale.set(sx,sy,1);scene.add(sprite);return {sprite,phase:rand()};}
const hazeLayers=[haze('#23465f',.28,-4.6,3.1,-5.6,8,4),haze('#1e5260',.2,3.8,2.3,-5.7,7,3.5),haze('#58705e',.12,-1.2,1.2,-5.4,6,2.7)];
const altar=createAltar();altar.position.set(2,0,-2);altar.scale.setScalar(1.4);scene.add(altar);
const well=new THREE.Group();well.position.set(fountain.x,0,fountain.z);scene.add(well);cylinder(.91,.99,.18,trim,well,0,.08,0,12);cylinder(.81,.89,.28,stone,well,0,.24,0,12);torus(.77,.10,trim,well,0,.4,0);
const waterMaterial=mat('#177780',{metalness:.65,roughness:.18,emissive:'#125a60',emissiveIntensity:.65,transparent:true,opacity:.9});const water=cylinder(.72,.72,.035,waterMaterial,well,0,.34,0,64);
for(let i=0;i<8;i++){const angle=i*Math.PI/4;const slab=roundedBox(.28,.12,.42,trim,well,Math.cos(angle)*.76,.39,Math.sin(angle)*.76,.04);slab.rotation.y=angle;}
cylinder(.16,.25,.55,trim,well,0,.58,0);cylinder(.37,.12,.13,gold,well,0,.89,0);cylinder(.32,.32,.035,waterMaterial,well,0,.96,0);
const glow=mat('#97ffed',{emissive:'#46e9ce',emissiveIntensity:3,roughness:.2});const drops=[];for(let i=0;i<100;i++){const d=sphere(.018,glow,well);drops.push({mesh:d,phase:rand(),angle:rand()*Math.PI*2,r:.24+rand()*.38});}
const ripples=[];for(let i=0;i<4;i++)ripples.push(torus(.15,.006,glow,well,0,.366,0));const springLight=new THREE.PointLight(0x48e8cf,8,5,2);springLight.position.set(0,1,-1);scene.add(springLight);
const flames=[];for(const x of [-3,3]){
 box(.17,.6,.2,gold,scene,x,1.1,-3.45);cylinder(.22,.1,.2,gold,scene,x,1.45,-3.4);
 const f=createFire(x+3);f.position.set(x,1.52,-3.4);f.scale.setScalar(1.5);scene.add(f);
 const light=new THREE.PointLight(0xffa450,13,8,2);light.position.set(x,1.9,-3);scene.add(light);flames.push({f,light,phase:rand()*5});
}
function knight(){const g=new THREE.Group();const body=new THREE.Group();g.add(body);const armor=mat('#58727b',{map:metalSurface.map,bumpMap:metalSurface.bump,bumpScale:.1,metalness:.82,roughness:.26}),armorLight=mat('#a7c2c5',{map:metalSurface.map,bumpMap:metalSurface.bump,bumpScale:.08,metalness:.76,roughness:.22}),boots=mat('#20292c',{roughness:.72}),leather=mat('#533c2f',{roughness:.9}),visor=mat('#101c20',{metalness:.5,roughness:.32}),accent=mat('#c8a45a',{metalness:.78,roughness:.26});const legs=[];for(const x of [-.14,.14]){const pivot=new THREE.Group();pivot.position.set(x,.49,0);body.add(pivot);sphere(.1,armor,pivot,0,-.14,0,.91,2,1);cylinder(.085,.09,.22,leather,pivot,0,-.30,0,16);
 roundedBox(.21,.065,.22,boots,pivot,0,-.205,0,.02);
 sphere(.115,leather,pivot,0,-.405,.074,.86,.58,1.48);
 sphere(.112,boots,pivot,0,-.455,.071,.89,.16,1.5);
 for(let i=0;i<3;i++){const crease=roundedBox(.15-i*.012,.009,.012,boots,pivot,0,-.345-i*.027,.099+i*.016,.004);crease.rotation.z=(i-1)*.06;}
 for(const side of [-1,1]){roundedBox(.014,.12,.014,accent,pivot,side*.074,-.287,.102,.004);}
 roundedBox(.16,.026,.013,boots,pivot,0,-.248,.112,.006);
 roundedBox(.036,.036,.018,accent,pivot,.05,-.248,.123,.006);
 const scuff=roundedBox(.068,.008,.015,accent,pivot,-.026,-.411,.228,.004);scuff.rotation.z=.13;legs.push(pivot);}
 const cuirass=new THREE.LatheGeometry([new THREE.Vector2(.145,.56),new THREE.Vector2(.17,.63),new THREE.Vector2(.22,.82),new THREE.Vector2(.215,.91),new THREE.Vector2(.13,.99)],24);
 const chest=mesh(cuirass,armor,body);chest.scale.z=.76;
 const belt=cylinder(.164,.153,.05,leather,body,0,.605,0,24);belt.scale.z=.78;
 roundedBox(.045,.045,.015,accent,body,0,.605,.135,.006);
 const skin=mat('#c3977e',{roughness:.86}),eyeWhite=mat('#ded8ca',{roughness:.7}),iris=mat('#384a42',{roughness:.6});
 cylinder(.11,.13,.14,skin,body,0,1.04,.02,12);
 const head=new THREE.Group();head.position.set(0,1.23,.02);body.add(head);
 sphere(.19,skin,head,0,0,.015,.88,1.13,.91);
 const helmet=new THREE.Group();head.add(helmet);helmet.visible=false;
 mesh(new THREE.SphereGeometry(.222,24,12,0,Math.PI*2,0,Math.PI*.48),armorLight,helmet,0,.035,-.015);
 const hair=new THREE.Group();head.add(hair);
 const hairDark=mat('#35251e',{roughness:1}),hairMid=mat('#493127',{roughness:.98}),hairLight=mat('#604132',{roughness:.97});
 // An uneven swept hairline, with fine raised strands following the same flow.
 const hairGeo=new THREE.SphereGeometry(.196,32,18,0,Math.PI*2,0,Math.PI*.61);
 const hp=hairGeo.attributes.position;
 for(let i=0;i<hp.count;i++){let x=hp.getX(i),y=hp.getY(i),z=hp.getZ(i);const front=Math.max(0,z/.196);y+=front*.045+Math.sin(x*17)*front*.014;hp.setXYZ(i,x*.91,y*1.04+.01,z*.94-.018);}hairGeo.computeVertexNormals();mesh(hairGeo,hairDark,hair);
 for(let i=0;i<19;i++){
  const a=(i/18-.5)*2.35;
  const path=new THREE.CatmullRomCurve3([
   new THREE.Vector3(Math.sin(a)*.171,.07+Math.cos(a)*.035,Math.cos(a)*.165-.018),
   new THREE.Vector3(Math.sin(a)*.15,.168+Math.cos(a)*.035,.025),
   new THREE.Vector3(Math.sin(a)*.10,.145,-.14),
   new THREE.Vector3(Math.sin(a)*.033,.036,-.197)]);
  mesh(new THREE.TubeGeometry(path,18,i%3===0?.008:.0045,5,false),i%3===0?hairMid:hairLight,hair);
 }
 const tiedHair=new THREE.CatmullRomCurve3([new THREE.Vector3(0,.035,-.193),new THREE.Vector3(.012,-.035,-.23),new THREE.Vector3(.022,-.14,-.24),new THREE.Vector3(.04,-.225,-.21)]);
 mesh(new THREE.TubeGeometry(tiedHair,16,.035,8,false),hairMid,hair);
 const hairTie=torus(.039,.007,leather,hair,0,.02,-.202);hairTie.rotation.x=.2;
 for(const side of [-1,1]){roundedBox(.052,.2,.12,armorLight,helmet,side*.175,-.025,-.03,.018);sphere(.024,eyeWhite,head,side*.067,.018,.166,1,.52,.45);sphere(.012,iris,head,side*.067,.018,.177,1,.85,.45);const brow=roundedBox(.063,.018,.017,leather,head,side*.068,.049,.164,.006);brow.rotation.z=-side*.12;}
 sphere(.032,skin,head,.008,-.024,.183,.7,1.3,.85);sphere(.066,skin,head,0,-.115,.11,1,.62,.8);
 const lips=mat('#895b50',{roughness:.94}),scar=mat('#dab19a',{roughness:1});
 const mouthCurve=new THREE.CatmullRomCurve3([new THREE.Vector3(-.04,-.085,.16),new THREE.Vector3(0,-.088,.17),new THREE.Vector3(.04,-.075,.16)]);mesh(new THREE.TubeGeometry(mouthCurve,12,.005,5,false),lips,head);
 for(const side of [-1,1]){const lid=roundedBox(.046,.008,.013,skin,head,side*.067,.03+(side<0?.006:0),.178,.004);lid.rotation.z=side*.08;}
 const nick=roundedBox(.005,.049,.006,scar,head,-.095,.045,.166,.002);nick.rotation.z=-.25;
 const plume=new THREE.Group();head.add(plume); // Stable legacy animation anchor, no spike.
 const browBand=mesh(new THREE.TorusGeometry(.215,.012,6,32,Math.PI),accent,helmet,0,.044,-.015);browBand.rotation.x=Math.PI/2;
 const ridge=new THREE.CatmullRomCurve3([new THREE.Vector3(0,.05,.205),new THREE.Vector3(0,.2,.1),new THREE.Vector3(0,.259,-.015),new THREE.Vector3(0,.2,-.13),new THREE.Vector3(0,.05,-.23)]);mesh(new THREE.TubeGeometry(ridge,20,.009,6,false),accent,helmet);
 for(let i=0;i<9;i++){const a=i*Math.PI/8;sphere(.009,accent,helmet,Math.cos(a)*.215,.055,Math.sin(a)*.215-.015);}
 function setHelmet(item){helmet.visible=!!item;hair.visible=!item;helmet.name=item?.name||'Unequipped helmet';}
 for(const side of [-1,1])for(let i=0;i<3;i++){
  const plate=roundedBox(.19-i*.015,.045,.255-i*.02,i?armor:armorLight,body,side*(.29+i*.04),.995-i*.059,0,.018);plate.rotation.z=-side*(.18+i*.16);
  for(const z of [-.09,.09])sphere(.012,accent,body,side*(.32+i*.04),.998-i*.059,z);
 }
 // Cloth starts at the collar and widens into folds below the shoulder blades.
 const cape=new THREE.Group();cape.position.set(0,1.045,-.13);body.add(cape);
 const capeGeo=new THREE.PlaneGeometry(1,1,16,18),cp=capeGeo.attributes.position;
 for(let i=0;i<cp.count;i++){const u=cp.getX(i),v=.5-cp.getY(i);cp.setXYZ(i,u*(.34+v*.4),-v*.86,-v*.13+Math.sin(u*30)*(.006+v*.024));}capeGeo.computeVertexNormals();
 const capeCloth=cloth.clone();capeCloth.side=THREE.DoubleSide;mesh(capeGeo,capeCloth,cape);
 for(const side of [-1,1]){const mantle=roundedBox(.18,.06,.29,capeCloth,body,side*.15,1.02,0,.025);mantle.rotation.z=side*.1;sphere(.026,accent,body,side*.16,1.02,.15,1,1,.45);}
 const clasp=new THREE.CatmullRomCurve3([new THREE.Vector3(-.16,1.02,.15),new THREE.Vector3(0,.99,.19),new THREE.Vector3(.16,1.02,.15)]);mesh(new THREE.TubeGeometry(clasp,12,.008,5,false),accent,body);
 const arm=new THREE.Group();arm.position.set(.34,.92,0);body.add(arm);cylinder(.085,.066,.25,armor,arm,0,-.12,0,16);
 const elbow=new THREE.Group();elbow.position.set(0,-.25,0);elbow.rotation.x=-.65;arm.add(elbow);sphere(.078,leather,elbow);cylinder(.075,.052,.22,armorLight,elbow,0,-.12,0,16);
 const wrist=new THREE.Group();wrist.position.set(0,-.25,0);elbow.add(wrist);
 const weaponSocket=new THREE.Group();weaponSocket.rotation.x=Math.PI/4+.65;wrist.add(weaponSocket);
 function grippingHand(parent){roundedBox(.105,.1,.072,skin,parent,0,0,-.03,.025);for(let i=0;i<4;i++)roundedBox(.082,.018,.056,skin,parent,.014,.032-i*.022,.012,.008);const thumb=roundedBox(.034,.065,.055,skin,parent,-.047,.018,.013,.012);thumb.rotation.z=-.35;}
 grippingHand(weaponSocket);
 let heldWeapon=null,heldKey;
 function setWeapon(item){const key=JSON.stringify(item??null);if(key===heldKey)return;heldKey=key;if(heldWeapon){heldWeapon.userData.dispose?.();weaponSocket.remove(heldWeapon);}heldWeapon=createHeldWeapon(item);weaponSocket.add(heldWeapon);}
 setWeapon({name:'long sword'});
 const shieldArm=new THREE.Group();shieldArm.position.set(-.34,.91,0);body.add(shieldArm);cylinder(.085,.066,.25,armor,shieldArm,0,-.13,0,16);
 const shieldElbow=new THREE.Group();shieldElbow.position.y=-.25;shieldElbow.rotation.x=-.9;shieldArm.add(shieldElbow);cylinder(.075,.052,.22,armorLight,shieldElbow,0,-.1,0,16);
 const shieldHand=new THREE.Group();shieldHand.position.set(0,-.23,0);shieldHand.rotation.set(.35,-.2,-.12);shieldElbow.add(shieldHand);grippingHand(shieldHand);
 const shieldRoot=new THREE.Group();shieldRoot.position.set(0,0,.085);shieldHand.add(shieldRoot);
 const shield=cylinder(.29,.29,.065,leather,shieldRoot,0,0,0,32);shield.rotation.x=Math.PI/2;
 const shieldFace=cylinder(.268,.268,.016,cloth,shieldRoot,0,0,.04,32);shieldFace.rotation.x=Math.PI/2;
 const shieldRim=torus(.278,.019,accent,shieldRoot,0,0,.049);shieldRim.rotation.x=0;
 sphere(.082,armorLight,shieldRoot,0,0,.065,1,1,.58);
 for(let i=0;i<12;i++){const a=i*Math.PI/6;sphere(.012,accent,shieldRoot,Math.cos(a)*.246,Math.sin(a)*.246,.056);}
 roundedBox(.035,.16,.035,leather,shieldRoot,0,0,-.055,.01);
 for(const y of [-.075,.075])roundedBox(.085,.022,.065,accent,shieldRoot,0,y,-.043,.008);
 return {g,body,legs,arm,elbow,wrist,weaponSocket,head,shieldArm,shieldElbow,cape,plume,setWeapon,setHelmet};}
function cat(){const g=new THREE.Group(),body=new THREE.Group();g.add(body);const fur=mat('#bd9361',{roughness:.94}),furLight=mat('#d3ad79',{roughness:.9}),paws=mat('#e1ca9e',{roughness:.88}),collar=mat('#ad7744',{metalness:.42,roughness:.44});sphere(.24,fur,body,0,.27,0,.75,.8,1.5);const head=sphere(.19,furLight,body,0,.46,.28);for(const x of [-.11,.11]){const e=mesh(new THREE.ConeGeometry(.085,.18,4),furLight,head,x,.16,0);e.rotation.z=x>0?-.2:.2;sphere(.025,glow,head,x*.65,.02,.16);}
 sphere(.035,mat('#d28c78',{roughness:.55}),head,0,-.015,.2);const collarRing=torus(.145,.018,collar,body,0,.34,.01);collarRing.scale.set(.9,1,1);const charm=sphere(.028,glow,body,0,.3,.16);const legs=[];for(const x of [-.12,.12])for(const z of [-.2,.2]){const leg=new THREE.Group();leg.position.set(x,.18,z);body.add(leg);roundedBox(.085,.2,.09,paws,leg,0,-.07,0,.025);legs.push(leg);}
 const tail=new THREE.Group();tail.position.set(0,.32,-.29);body.add(tail);const curve=new THREE.CatmullRomCurve3([new THREE.Vector3(0,0,0),new THREE.Vector3(0,.2,-.14),new THREE.Vector3(0,.48,-.17),new THREE.Vector3(.11,.56,-.14)]);mesh(new THREE.TubeGeometry(curve,16,.038,10,false),fur,tail);const whiskers=[];for(const side of [-1,1]){const whisker=new THREE.CatmullRomCurve3([new THREE.Vector3(side*.05,-.02,.19),new THREE.Vector3(side*.18,-.03,.26),new THREE.Vector3(side*.28,-.01,.23)]);whiskers.push(mesh(new THREE.TubeGeometry(whisker,8,.008,5,false),furLight,head));}
 return {g,body,legs,tail,collarRing,charm};}
function sentinel(){const g=new THREE.Group();const core=mat('#ff9a58',{emissive:'#ff4d1f',emissiveIntensity:5,roughness:.3}),guard=mat('#3d4b4b',{map:stoneSurface.map,bumpMap:stoneSurface.bump,bumpScale:.14,roughness:.8}),guardLight=mat('#77827a',{map:stoneSurface.map,bumpMap:stoneSurface.bump,bumpScale:.1,roughness:.65}),visor=mat('#161f22',{metalness:.5,roughness:.3});cylinder(.38,.45,.18,darkStone,g,0,.09,0,8);roundedBox(.53,.7,.43,guard,g,0,.52,0,.08);roundedBox(.3,.3,.46,darkStone,g,0,.78,.03,.04);for(const x of [-.43,.43]){roundedBox(.3,.2,.38,guardLight,g,x,.72,0,.06);roundedBox(.18,.42,.25,guard,g,x,.45,0,.05);}
 sphere(.27,guardLight,g,0,1.03,0,1,.9,1);roundedBox(.32,.09,.05,visor,g,0,1.04,.25,.02);for(const x of [-.14,.14]){const horn=mesh(new THREE.ConeGeometry(.07,.25,5),guardLight,g,x,1.3,0);horn.rotation.z=x>0?.35:-.35;}
 const coreMesh=sphere(.1,core,g,0,.59,.25);roundedBox(.07,.37,.05,core,g,0,.59,.3,.015);return Object.assign(g,{userData:{core:coreMesh}});}
const player=knight(),pet=cat(),enemy=sentinel();scene.add(player.g,pet.g,enemy);
const $=s=>document.querySelector(s);
function galleryLabel(text){const canvas=document.createElement('canvas');canvas.width=384;canvas.height=64;const ctx=canvas.getContext('2d');ctx.fillStyle='rgba(8,15,22,.72)';ctx.beginPath();ctx.roundRect(18,10,348,44,9);ctx.fill();ctx.font='22px system-ui';ctx.textAlign='center';ctx.fillStyle='#e8ddc7';ctx.fillText(text,192,39);const texture=new THREE.CanvasTexture(canvas),material=new THREE.SpriteMaterial({map:texture,depthTest:false});const sprite=new THREE.Sprite(material);sprite.scale.set(1.05,.18,1);sprite.position.y=1.48;sprite.userData.dispose=()=>{texture.dispose();material.dispose();};return sprite;}
const gallery=new THREE.Group();const galleryActors=[];const galleryNames=['jackal','fox','coyote','newt','gecko','kobold','large kobold','lichen','yellow mold','floating eye','acid blob','yellow light','giant bat','killer bee','giant ant','cave spider','centipede','garter snake','kobold zombie','homunculus','hobbit','gnome','hill orc','shopkeeper'];for(const [i,name] of galleryNames.entries()){const actor=createCreature({name,kind:'monster'});stageCreature(actor.g,{disposition:name==='shopkeeper'?'peaceful':'hostile'});actor.g.position.set((i%6-2.5)*1.7,0,(Math.floor(i/6)-1.5)*1.7);actor.g.add(galleryLabel(name));gallery.add(actor.g);galleryActors.push({actor,phase:i*.73});}gallery.visible=false;scene.add(gallery);
const galleryButton=document.createElement('button');galleryButton.id='gallery';galleryButton.textContent='Creature gallery';$('.buttons').append(galleryButton);function setGallery(value){gallery.visible=value;for(const object of [player.g,pet.g,enemy,marker])object.visible=!value;galleryButton.textContent=value?'Close gallery':'Creature gallery';if(value)$('#message').textContent='Creature test roster: silhouettes and idle animations';else $('#message').textContent=state.message;}galleryButton.onclick=()=>setGallery(!gallery.visible);
const marker=torus(.4,.013,mat('#d9c58e',{emissive:'#a18843',emissiveIntensity:.5}));marker.position.y=.025;
const motes=[];const moteMaterial=mat('#c6c598',{emissive:'#9bba87',emissiveIntensity:1});for(let i=0;i<45;i++){const m=sphere(.009,moteMaterial);motes.push({m,x:rand()*9-4.5,z:rand()*7-3.5,y:rand()*3,phase:rand()*6});}
let state=createState(),lastTime=0,attackTime=-10,moveTime=-10,lockedUntil=0;let audioContext,osc,gain;
function updateHUD(){$('#hp').textContent=`${state.hp} / 24`;$('#healthbar').style.width=`${state.hp/24*100}%`;$('#turn').textContent=state.turn;$('#message').textContent=state.message;}
function reset(){state=createState();player.g.position.set(state.player.x,0,state.player.z);pet.g.position.set(state.cat.x,0,state.cat.z);enemy.position.set(state.enemy.x,0,state.enemy.z);enemy.scale.setScalar(1);enemy.visible=true;attackTime=-10;lockedUntil=0;updateHUD();}reset();
function action(a){const now=performance.now()/1000;if(now<lockedUntil)return;const result=act(state,a);if(result.type!=='blocked')lockedUntil=now+.18;if(result.type==='attack'){attackTime=now;player.g.rotation.y=Math.atan2(state.enemy.x-state.player.x,state.enemy.z-state.player.z);}if(result.type==='move'){moveTime=now;player.g.rotation.y=Math.atan2(a.dx,a.dz);}updateHUD();}
const live=installLive({scene,camera,controls,playerFactory:knight,catFactory:cat,monsterFactory:sentinel,creatureFactory:createCreature,wellTemplate:well,onDemo:()=>{$('.stats').innerHTML='<span>AC <b>−2</b></span><span>LVL <b>1</b></span><span>TURN <b id="turn">0</b></span>';updateHUD();},onMode:active=>{galleryButton.hidden=active;setGallery(false);if(active)for(const object of [player.g,pet.g,enemy,marker])object.visible=false;},demoObjects:scene.children.filter(o=>!o.isHemisphereLight&&!o.isDirectionalLight)});
const keys={k:[0,-1],ArrowUp:[0,-1],j:[0,1],ArrowDown:[0,1],h:[-1,0],ArrowLeft:[-1,0],l:[1,0],ArrowRight:[1,0],y:[-1,-1],u:[1,-1],b:[-1,1],n:[1,1]};
addEventListener('keydown',e=>{if(live.active || e.target instanceof HTMLButtonElement || e.metaKey || e.ctrlKey || e.altKey)return;const key=e.key.length===1?e.key.toLowerCase():e.key;if(keys[key]){e.preventDefault();action({type:'move',dx:keys[key][0],dz:keys[key][1]});}else if(key===' '){e.preventDefault();action({type:'wait'});}else if(key==='e')action({type:'drink'});});
$('#reset').onclick=reset;$('#camera').onclick=()=>{const p=camera.position.clone().sub(controls.target);p.applyAxisAngle(new THREE.Vector3(0,1,0),Math.PI/2);camera.position.copy(p.add(controls.target));};
$('#sound').onclick=async()=>{if(!audioContext){audioContext=new AudioContext();osc=audioContext.createOscillator();gain=audioContext.createGain();osc.type='sine';osc.frequency.value=110;gain.gain.value=.025;osc.connect(gain);gain.connect(audioContext.destination);osc.start();await audioContext.resume();}else if(audioContext.state==='running')await audioContext.suspend();else await audioContext.resume();const active=audioContext.state==='running';$('#sound').textContent=active?'Sound on':'Sound off';$('#sound').setAttribute('aria-pressed',String(active));};
addEventListener('resize',()=>{camera.aspect=innerWidth/innerHeight;camera.updateProjectionMatrix();renderer.setSize(innerWidth,innerHeight);composer.setSize(innerWidth,innerHeight);});
renderer.setAnimationLoop(ms=>{const t=ms/1000,dt=Math.min(t-lastTime,.05);lastTime=t;const mix=1-Math.exp(-dt*14);
 player.g.position.x=THREE.MathUtils.lerp(player.g.position.x,state.player.x,mix);player.g.position.z=THREE.MathUtils.lerp(player.g.position.z,state.player.z,mix);
 const moving=Math.hypot(player.g.position.x-state.player.x,player.g.position.z-state.player.z)>.025;
 player.body.position.y=Math.sin(t*(moving?18:2))*(moving?.035:.013);player.legs.forEach((l,i)=>l.rotation.x=moving?Math.sin(t*18+i*Math.PI)*.5:0);player.cape.rotation.x=-.17+Math.sin(t*3)*.06;player.plume.rotation.z=-.16+Math.sin(t*2.4)*.035;
 const swing=Math.max(0,1-(t-attackTime)/.35);player.arm.rotation.x=-Math.sin(swing*Math.PI)*1.9;player.wrist.rotation.x=Math.sin(swing*Math.PI)*.34;player.weaponSocket.rotation.z=-Math.sin(swing*Math.PI)*1.18;player.shieldArm.rotation.z=Math.sin(swing*Math.PI)*.12;
 const pdx=state.cat.x-pet.g.position.x,pdz=state.cat.z-pet.g.position.z,petMoving=Math.hypot(pdx,pdz)>.04;if(petMoving)pet.g.rotation.y=Math.atan2(pdx,pdz);pet.g.position.x+=pdx*mix*.7;pet.g.position.z+=pdz*mix*.7;pet.body.position.y=Math.sin(t*(petMoving?22:2.5))*.015;pet.legs.forEach((l,i)=>l.rotation.x=petMoving?Math.sin(t*22+i*2)*.4:0);pet.tail.rotation.z=Math.sin(t*3)*.24;pet.charm.position.y=.3+Math.sin(t*4)*.025;
 if(gallery.visible)for(const {actor,phase} of galleryActors){const quirk=actor.quirk;actor.body.position.y=Math.sin(t*(quirk==='dragon'?2.1:2.5)+phase)*(quirk==='dragon'?.035:quirk==='gridbug'?.008:.018);actor.legs.forEach((l,i)=>l.rotation.x=Math.sin(t*(quirk==='gridbug'?9:2.2)+i+phase)*.08);if(actor.tail)actor.tail.rotation.z=Math.sin(t*(quirk==='dog'?7:quirk==='unicorn'?2.6:3)+phase)*(quirk==='dog'?.34:quirk==='unicorn'?.16:.24);if(actor.wings?.length)actor.wings.forEach((wing,i)=>{if(quirk==='bat')wing.rotation.z=(wing.userData.side||(i?1:-1))*Math.sin(t*14+phase)*.65;else if(quirk==='bee')wing.rotation.y=(i?1:-1)*Math.sin(t*60)*.35;else wing.rotation.y=(i?1:-1)*(-.18+Math.sin(t*5+phase)*.12);});if(quirk==='hover'||quirk==='bat'||quirk==='bee')actor.body.position.y=Math.sin(t*2.2+phase)*.06;if(quirk==='dragon')actor.g.rotation.z=Math.sin(t*1.7+phase)*.025;if(actor.g.userData.core)actor.g.userData.core.material.emissiveIntensity=4.5+Math.sin(t*5+phase)*1.4;}
 marker.position.set(player.g.position.x,.025,player.g.position.z);enemy.rotation.y=Math.atan2(player.g.position.x-enemy.position.x,player.g.position.z-enemy.position.z);enemy.position.y=state.enemy.hp>0?Math.sin(t*2)*.02:0;if(state.enemy.hp<=0){enemy.scale.multiplyScalar(Math.exp(-dt*6));if(enemy.scale.x<.02)enemy.visible=false;}
 for(const d of drops){const p=(t*.8+d.phase)%1;d.mesh.position.set(Math.cos(d.angle)*d.r*p,.96+Math.sin(p*Math.PI)*.55-p*.6,Math.sin(d.angle)*d.r*p);d.mesh.scale.setScalar(.6+Math.sin(p*Math.PI)*.6);}
 ripples.forEach((r,i)=>{const p=(t*.45+i/4)%1;r.scale.setScalar(1+p*3.3);r.position.y=.365+Math.sin(t*3+i)*.004;});water.rotation.y=t*.1;
 flames.forEach(({f,light,phase})=>{f.userData.updateFire(t);light.intensity=13+Math.sin(t*11+phase)*1.3+Math.sin(t*19.3+phase)*.6;});
 for(const m of motes)m.m.position.set(m.x+Math.sin(t*.3+m.phase)*.2,(m.y+t*.055)%3.5,m.z+Math.cos(t*.2+m.phase)*.15);
 hazeLayers.forEach(({sprite,phase},i)=>{sprite.position.x+=Math.sin(t*.11+phase)*.0015;sprite.position.y+=Math.cos(t*.17+phase)*.0008;sprite.material.opacity=(.12+i*.035)+Math.sin(t*.23+phase)*.025;});enemy.userData.core.material.emissiveIntensity=4.5+Math.sin(t*5)*1.4;
 if(gain&&audioContext.state==='running')gain.gain.value=.02+Math.sin(t*.7)*.004;
 live.update(t,dt);controls.update();composer.render();
});
// Read-only state snapshot for smoke tests and future engine-adapter experiments.
window.roomPrototype={snapshot:()=>structuredClone(state),rendererInfo:()=>({calls:renderer.info.render.calls,triangles:renderer.info.render.triangles})};
