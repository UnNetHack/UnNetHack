import * as THREE from 'three';
import {RoundedBoxGeometry} from 'three/addons/geometries/RoundedBoxGeometry.js';

const M={
 skin:new THREE.MeshStandardMaterial({color:0xb78f72,roughness:.9}),greenSkin:new THREE.MeshStandardMaterial({color:0x63764b,roughness:.92}),graySkin:new THREE.MeshStandardMaterial({color:0x8b8374,roughness:.9}),fur:new THREE.MeshStandardMaterial({color:0xb98a5b,roughness:.94}),whiteFur:new THREE.MeshStandardMaterial({color:0xd6d2c1,roughness:.9}),
 cloth:new THREE.MeshStandardMaterial({color:0x315b59,roughness:.94}),redCloth:new THREE.MeshStandardMaterial({color:0x743b3c,roughness:.9}),brownCloth:new THREE.MeshStandardMaterial({color:0x68452f,roughness:.92}),blueCloth:new THREE.MeshStandardMaterial({color:0x3d5278,roughness:.9}),
 steel:new THREE.MeshStandardMaterial({color:0x91a8aa,metalness:.76,roughness:.3}),darkSteel:new THREE.MeshStandardMaterial({color:0x39484b,metalness:.7,roughness:.38}),gold:new THREE.MeshStandardMaterial({color:0xb9954d,metalness:.78,roughness:.3}),leather:new THREE.MeshStandardMaterial({color:0x493228,roughness:.9}),beard:new THREE.MeshStandardMaterial({color:0x9a5b35,roughness:.96}),
 eye:new THREE.MeshStandardMaterial({color:0xffb66b,emissive:0xd95b1e,emissiveIntensity:2.5,roughness:.24}),electric:new THREE.MeshStandardMaterial({color:0x5d91b1,emissive:0x1e91ca,emissiveIntensity:1.8,roughness:.34}),fire:new THREE.MeshStandardMaterial({color:0xff8750,emissive:0xf04a18,emissiveIntensity:4,roughness:.3}),wing:new THREE.MeshStandardMaterial({color:0x4c3032,roughness:.86,side:THREE.DoubleSide}),
};
function part(parent,geometry,material,x=0,y=0,z=0){const mesh=new THREE.Mesh(geometry,material);mesh.position.set(x,y,z);mesh.castShadow=mesh.receiveShadow=true;parent.add(mesh);return mesh;}
function rounded(parent,w,h,d,material,x=0,y=0,z=0,r=.04){return part(parent,new RoundedBoxGeometry(w,h,d,3,r),material,x,y,z);}
function sphere(parent,r,material,x=0,y=0,z=0,sx=1,sy=1,sz=1){const mesh=part(parent,new THREE.SphereGeometry(r,16,12),material,x,y,z);mesh.scale.set(sx,sy,sz);return mesh;}
function cylinder(parent,r1,r2,h,material,x=0,y=0,z=0,segments=12){return part(parent,new THREE.CylinderGeometry(r1,r2,h,segments),material,x,y,z);}
function cone(parent,r,h,material,x=0,y=0,z=0,segments=6){return part(parent,new THREE.ConeGeometry(r,h,segments),material,x,y,z);}
function actor(g,body,legs=[],tail=null,wings=[],quirk='idle'){return {g,body,legs,tail,wings,quirk};}
function eyes(head,material=M.eye,y=0,z=.18,spread=.075){for(const x of [-spread,spread])sphere(head,.026,material,x,y,z);}
function humanoid(kind){
 const g=new THREE.Group(),body=new THREE.Group();g.add(body);const legs=[];
 const short=kind==='gnome',stocky=kind==='orc'||kind==='dwarf',guard=kind==='guard',shopkeeper=kind==='shopkeeper';
 const height=short?.72:guard?1.12:stocky?.9:1;const skin=kind==='orc'?M.greenSkin:kind==='dwarf'?M.graySkin:M.skin;
 for(const x of [-.13,.13]){const leg=new THREE.Group();leg.position.set(x,.4,0);body.add(leg);rounded(leg,.16,short?.27:stocky?.34:.42,.16,M.darkSteel,0,-.12,0,.035);rounded(leg,.21,.13,.28,M.leather,0,-.36,.06,.03);legs.push(leg);}
 rounded(body,stocky?.46:.42,short?.3:stocky?.4:.48,.3,kind==='orc'?M.brownCloth:shopkeeper?M.brownCloth:kind==='guard'?M.steel:M.cloth,0,.62,0,.06);sphere(body,short?.18:.22,skin,0,short?.87:1.0,.02,1,1.05,1);
 if(kind==='gnome'){const cap=cone(body,.25,.36,M.redCloth,0,1.2,.01,8);cap.rotation.z=-.16;sphere(body,.19,M.beard,0,.86,.18,.8,.9,.65);sphere(body,.05,skin,0,.98,.19,1,1,.8);}
 if(kind==='orc'){for(const x of [-.09,.09]){const tusk=cone(body,.045,.15,M.whiteFur,x,.91,.19,5);tusk.rotation.x=x<0?.35:-.35;}for(const x of [-.31,.31])sphere(body,.16,M.darkSteel,x,.84,0,1,.75,1);}
 if(kind==='dwarf'){cylinder(body,.22,.25,.15,M.darkSteel,0,1.17,0,10);const beard=sphere(body,.2,M.beard,0,1.0,.18,.95,1.1,.6);beard.scale.y=1.25;}
 if(guard){cylinder(body,.23,.23,.13,M.darkSteel,0,1.19,0,10);const plume=cone(body,.06,.25,M.redCloth,0,1.38,-.01,6);plume.rotation.z=-.12;rounded(body,.48,.07,.32,M.gold,0,.78,0,.02);}
 if(shopkeeper){rounded(body,.19,.26,.07,M.leather,.28,.67,.16,.025);const hat=cylinder(body,.25,.2,.13,M.brownCloth,0,1.2,0,12);hat.rotation.x=.04;}
 eyes(body,kind==='orc'?M.fire:M.eye,short?.91:1.04,.205,.075);
 if(guard){const spear=rounded(body,.045,.7,.045,M.steel,.36,.7,.24,.01);spear.rotation.z=-.12;cone(body,.07,.14,M.steel,.36,1.1,.24,5).rotation.x=Math.PI;}
 if(kind==='dwarf'){const pick=rounded(body,.045,.55,.045,M.steel,-.38,.67,.18,.01);pick.rotation.z=.55;const head=rounded(body,.26,.05,.05,M.steel,-.38,.94,.18,.01);head.rotation.z=-.2;}
 return actor(g,body,legs,null,[],kind);
}
function dog(){
 const g=new THREE.Group(),body=new THREE.Group();g.add(body);const legs=[];sphere(body,.25,M.fur,0,.31,0,.9,.75,1.4);const head=sphere(body,.19,M.fur,0,.47,.25,.95,1,1);sphere(body,.1,M.leather,0,.43,.4,.8,.65,.7);for(const x of [-.1,.1]){const ear=cone(head,.075,.16,M.fur,x,.16,.01,5);ear.rotation.z=x>0?-.45:.45;eyes(head,M.eye,.02,.16,.06);}for(const x of [-.13,.13])for(const z of [-.18,.18]){const leg=new THREE.Group();leg.position.set(x,.2,z);body.add(leg);rounded(leg,.08,.22,.09,M.fur,0,-.1,0,.025);legs.push(leg);}
 const tail=new THREE.Group();tail.position.set(0,.37,-.27);body.add(tail);const curve=new THREE.CatmullRomCurve3([new THREE.Vector3(0,0,0),new THREE.Vector3(0,.18,-.1),new THREE.Vector3(.1,.32,-.12)]);part(tail,new THREE.TubeGeometry(curve,12,.035,8,false),M.fur);return actor(g,body,legs,tail,[],'dog');
}
function gridBug(){
 const g=new THREE.Group(),body=new THREE.Group();g.add(body);const legs=[];sphere(body,.17,M.electric,0,.25,0,.8,.6,1.25);sphere(body,.11,M.darkSteel,0,.27,.16,.9,.72,1);eyes(body,M.electric,.01,.15,.055);
 for(const [x,z] of [[-.16,-.12],[-.19,0],[-.16,.12],[.16,-.12],[.19,0],[.16,.12]]){const leg=new THREE.Group();leg.position.set(x,.25,z);body.add(leg);const limb=rounded(leg,.035,.22,.035,M.darkSteel,0,-.02,x<0?-.08:.08,.01);limb.rotation.z=x<0?-.55:.55;legs.push(leg);}
 for(const x of [-.06,.06]){const antenna=rounded(body,.018,.16,.018,M.electric,x,.39,.19,.005);antenna.rotation.x=x<0?-.28:.28;}
 return actor(g,body,legs,null,[],'gridbug');
}
function unicorn(){
 const g=new THREE.Group(),body=new THREE.Group();g.add(body);const legs=[];sphere(body,.3,M.whiteFur,0,.43,0,.8,.72,1.25);cylinder(body,.12,.19,.42,M.whiteFur,0,.75,.12,10);const head=sphere(body,.19,M.whiteFur,0,1.02,.25,.9,1,1);const horn=cone(head,.055,.3,M.gold,0,.22,.06,5);horn.rotation.z=-.1;for(const x of [-.1,.1]){const ear=cone(head,.06,.15,M.whiteFur,x,.18,.02,5);ear.rotation.z=x>0?-.25:.25;}eyes(head,M.eye,.02,.18,.06);for(const x of [-.13,.13])for(const z of [-.17,.17]){const leg=new THREE.Group();leg.position.set(x,.27,z);body.add(leg);rounded(leg,.09,.34,.1,M.whiteFur,0,-.15,0,.025);rounded(leg,.1,.08,.11,M.gold,0,-.32,0,.02);legs.push(leg);}
 const tail=new THREE.Group();tail.position.set(0,.48,-.3);body.add(tail);const curve=new THREE.CatmullRomCurve3([new THREE.Vector3(0,0,0),new THREE.Vector3(0,.16,-.12),new THREE.Vector3(-.1,.28,-.18)]);part(tail,new THREE.TubeGeometry(curve,12,.045,8,false),M.whiteFur);return actor(g,body,legs,tail,[],'unicorn');
}
function dragon(){
 const g=new THREE.Group(),body=new THREE.Group();g.add(body);const legs=[];const scale=1.12;g.scale.setScalar(scale);sphere(body,.34,M.greenSkin,0,.5,0,1.2,.8,1.45);const head=sphere(body,.24,M.greenSkin,0,.84,.28,1.05,.9,1);sphere(body,.13,M.fire,0,.8,.47,.9,.65,.65);for(const x of [-.12,.12]){const horn=cone(head,.07,.25,M.darkSteel,x,.2,.03,5);horn.rotation.z=x>0?.35:-.35;}eyes(head,M.fire,.02,.21,.08);for(const x of [-.22,.22])for(const z of [-.17,.17]){const leg=new THREE.Group();leg.position.set(x,.3,z);body.add(leg);rounded(leg,.13,.3,.14,M.greenSkin,0,-.12,0,.035);cone(leg,.1,.1,M.darkSteel,0,-.28,.02,5).rotation.x=Math.PI;legs.push(leg);}
 const wings=[];for(const x of [-1,1]){const shape=new THREE.Shape();shape.moveTo(0,0);shape.lineTo(x*.55,.15);shape.lineTo(x*.42,.62);shape.lineTo(x*.15,.4);shape.lineTo(0,.12);const wing=part(body,new THREE.ShapeGeometry(shape),M.wing,x*.3,.72,-.02);wing.rotation.y=x>0?.18:-.18;wings.push(wing);}
 const tail=new THREE.Group();tail.position.set(0,.48,-.38);body.add(tail);const curve=new THREE.CatmullRomCurve3([new THREE.Vector3(0,0,0),new THREE.Vector3(0,.08,-.22),new THREE.Vector3(.18,.16,-.46)]);part(tail,new THREE.TubeGeometry(curve,12,.07,8,false),M.greenSkin);const core=sphere(body,.1,M.fire,0,.55,.31);g.userData.core=core;return Object.assign(actor(g,body,legs,tail,wings,'dragon'),{core});
}
function rat(giant=false){
 const g=new THREE.Group(),body=new THREE.Group(),legs=[];g.add(body);g.scale.setScalar(giant?1.25:.85);
 sphere(body,.22,M.graySkin,0,.24,-.04,1,.85,1.45);
 sphere(body,.16,M.leather,0,.28,.2,.85,.8,1.2);
 sphere(body,.09,M.graySkin,0,.24,.35,.85,.7,1.25);
 sphere(body,.035,M.skin,0,.25,.445,1,.7,.65);
 for(const side of [-1,1]){
  sphere(body,.095,M.graySkin,side*.115,.405,.17,1,1,.38);
  sphere(body,.065,M.skin,side*.115,.41,.201,1,1,.18);
  sphere(body,.023,M.leather,side*.101,.31,.3);
  sphere(body,.009,M.whiteFur,side*.106,.319,.316);
  rounded(body,.024,.05,.022,M.whiteFur,side*.018,.192,.416,.006);
  for(const offset of [-1,0,1]){
   const curve=new THREE.CatmullRomCurve3([new THREE.Vector3(side*.06,.24,.37),new THREE.Vector3(side*.17,.25+offset*.02,.38),new THREE.Vector3(side*.26,.25+offset*.03,.36+offset*.04)]);
   part(body,new THREE.TubeGeometry(curve,5,.003,3,false),M.whiteFur);
  }
  for(const z of [-.19,.17]){const leg=new THREE.Group();leg.position.set(side*.15,.13,z);body.add(leg);sphere(leg,.065,M.graySkin,0,-.02,0,.7,1,.9);rounded(leg,.075,.035,.12,M.skin,0,-.09,.04,.012);legs.push(leg);}
 }
 const tail=new THREE.Group();tail.position.set(0,.22,-.31);body.add(tail);
 const curve=new THREE.CatmullRomCurve3([new THREE.Vector3(),new THREE.Vector3(.08,-.12,-.16),new THREE.Vector3(.25,-.17,-.28),new THREE.Vector3(.33,-.16,-.46)]);
 for(let i=0;i<14;i++){const start=curve.getPoint(i/14),end=curve.getPoint((i+1)/14),direction=end.clone().sub(start),radius=.027*(1-i/15);const segment=part(tail,new THREE.CylinderGeometry(radius*.86,radius,direction.length(),8),i%2?M.skin:M.beard);segment.position.copy(start.add(end).multiplyScalar(.5));segment.quaternion.setFromUnitVectors(new THREE.Vector3(0,1,0),direction.normalize());}
 return actor(g,body,legs,tail,[],'rat');
}
function guardian(){const g=new THREE.Group(),body=new THREE.Group();g.add(body);rounded(body,.42,.78,.38,M.darkSteel,0,.5,0,.07);sphere(body,.23,M.graySkin,0,1.03,0,1,.9,1);for(const x of [-.4,.4])rounded(body,.25,.5,.3,M.steel,x,.58,0,.05);const core=sphere(body,.09,M.fire,0,.62,.23);g.userData.core=core;eyes(body,M.fire,1.04,.22,.08);return Object.assign(actor(g,body),{core});}
export function createCreature(cell={}){const name=(cell.name||'').toLowerCase();if(/^(sewer rat|giant rat|rabid rat|rat)$/.test(name))return rat(name==='giant rat');if(/grid ?bug/.test(name))return gridBug();if(/dragon/.test(name))return dragon();if(/unicorn/.test(name))return unicorn();if(/dog|hound|wolf/.test(name))return dog();if(/shopkeeper|merchant/.test(name))return humanoid('shopkeeper');if(/guard|soldier|watchman/.test(name))return humanoid('guard');if(/orc/.test(name))return humanoid('orc');if(/dwarf/.test(name))return humanoid('dwarf');if(/gnome/.test(name))return humanoid('gnome');return guardian();}
