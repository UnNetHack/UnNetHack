import * as THREE from 'three';
import {RoundedBoxGeometry} from 'three/addons/geometries/RoundedBoxGeometry.js';

const M={
 skin:new THREE.MeshStandardMaterial({color:0xb78f72,roughness:.9}),greenSkin:new THREE.MeshStandardMaterial({color:0x63764b,roughness:.92}),graySkin:new THREE.MeshStandardMaterial({color:0x8b8374,roughness:.9}),fur:new THREE.MeshStandardMaterial({color:0xb98a5b,roughness:.94}),whiteFur:new THREE.MeshStandardMaterial({color:0xd6d2c1,roughness:.9}),
 cloth:new THREE.MeshStandardMaterial({color:0x315b59,roughness:.94}),redCloth:new THREE.MeshStandardMaterial({color:0x743b3c,roughness:.9}),brownCloth:new THREE.MeshStandardMaterial({color:0x68452f,roughness:.92}),blueCloth:new THREE.MeshStandardMaterial({color:0x3d5278,roughness:.9}),
 steel:new THREE.MeshStandardMaterial({color:0x91a8aa,metalness:.76,roughness:.3}),darkSteel:new THREE.MeshStandardMaterial({color:0x39484b,metalness:.7,roughness:.38}),gold:new THREE.MeshStandardMaterial({color:0xb9954d,metalness:.78,roughness:.3}),leather:new THREE.MeshStandardMaterial({color:0x493228,roughness:.9}),beard:new THREE.MeshStandardMaterial({color:0x9a5b35,roughness:.96}),
 deadEye:new THREE.MeshStandardMaterial({color:0xcfe8c0,emissive:0x6fa860,emissiveIntensity:1.2,roughness:.3}),eye:new THREE.MeshStandardMaterial({color:0xffb66b,emissive:0xd95b1e,emissiveIntensity:2.5,roughness:.24}),electric:new THREE.MeshStandardMaterial({color:0x5d91b1,emissive:0x1e91ca,emissiveIntensity:1.8,roughness:.34}),fire:new THREE.MeshStandardMaterial({color:0xff8750,emissive:0xf04a18,emissiveIntensity:4,roughness:.3}),wing:new THREE.MeshStandardMaterial({color:0x4c3032,roughness:.86,side:THREE.DoubleSide}),
};
function part(parent,geometry,material,x=0,y=0,z=0){const mesh=new THREE.Mesh(geometry,material);mesh.position.set(x,y,z);mesh.castShadow=mesh.receiveShadow=true;parent.add(mesh);return mesh;}
function rounded(parent,w,h,d,material,x=0,y=0,z=0,r=.04){return part(parent,new RoundedBoxGeometry(w,h,d,3,r),material,x,y,z);}
function sphere(parent,r,material,x=0,y=0,z=0,sx=1,sy=1,sz=1){const mesh=part(parent,new THREE.SphereGeometry(r,16,12),material,x,y,z);mesh.scale.set(sx,sy,sz);return mesh;}
function cylinder(parent,r1,r2,h,material,x=0,y=0,z=0,segments=12){return part(parent,new THREE.CylinderGeometry(r1,r2,h,segments),material,x,y,z);}
function cone(parent,r,h,material,x=0,y=0,z=0,segments=6){return part(parent,new THREE.ConeGeometry(r,h,segments),material,x,y,z);}
function actor(g,body,legs=[],tail=null,wings=[],quirk='idle'){return {g,body,legs,tail,wings,quirk};}
function eyes(head,material=M.eye,y=0,z=.18,spread=.075){for(const x of [-spread,spread])sphere(head,.026,material,x,y,z);}
function humanoid(kind,o={}){
 const g=new THREE.Group(),body=new THREE.Group();g.add(body);const legs=[],wings=[];
 const short=['gnome','kobold','hobbit','imp'].includes(kind),stocky=kind==='orc'||kind==='dwarf',guard=kind==='guard',shopkeeper=kind==='shopkeeper',undead=kind==='zombie'||kind==='mummy';
 const skin=o.skin||(kind==='orc'?M.greenSkin:kind==='dwarf'?M.graySkin:M.skin);
 const torso=o.cloth||(kind==='orc'||shopkeeper?M.brownCloth:guard?M.steel:M.cloth);
 const headY=short?.87:1.0,shoulderY=short?.7:.8,torsoW=stocky?.46:.42;
 for(const x of [-.13,.13]){const leg=new THREE.Group();leg.position.set(x,.4,0);body.add(leg);rounded(leg,.16,short?.27:stocky?.34:.42,.16,kind==='mummy'?torso:M.darkSteel,0,-.12,0,.035);rounded(leg,.21,.13,.28,kind==='imp'||kind==='kobold'?skin:M.leather,0,-.36,.06,.03);legs.push(leg);}
 rounded(body,torsoW,short?.3:stocky?.4:.48,.3,torso,0,.62,0,.06);sphere(body,short?.18:.22,skin,0,headY,.02,1,1.05,1);
 // arms give every humanoid a readable silhouette; the undead reach forward
 for(const side of [-1,1]){const arm=new THREE.Group();arm.position.set(side*(torsoW/2+.07),shoulderY,0);body.add(arm);rounded(arm,.11,short?.3:.38,.12,undead?skin:torso,0,short?-.13:-.17,0,.03);sphere(arm,.065,skin,0,short?-.3:-.38,0);if(undead){arm.rotation.x=-1.35;arm.rotation.z=side*.08;}else arm.rotation.z=side*.12;}
 if(undead)body.rotation.x=.14;
 if(kind==='gnome'){const cap=cone(body,.25,.36,o.cap||M.redCloth,0,1.2,.01,8);cap.rotation.z=-.16;sphere(body,.19,M.beard,0,.86,.18,.8,.9,.65);sphere(body,.05,skin,0,.98,.19,1,1,.8);}
 if(kind==='kobold'){const snout=cone(body,.1,.2,skin,0,.83,.24,6);snout.rotation.x=Math.PI/2;sphere(body,.025,M.leather,0,.83,.34);for(const side of [-1,1]){const ear=cone(body,.07,.26,skin,side*.2,.95,-.01,4);ear.rotation.z=-side*1.15;}const spear=rounded(body,.035,.9,.035,M.leather,.34,.62,.2,.01);spear.rotation.x=.15;cone(body,.05,.14,M.darkSteel,.34,1.08,.27,4);}
 if(kind==='hobbit'){sphere(body,.2,M.beard,0,.95,-.02,1,.7,1);for(const side of [-1,1])rounded(body,.14,.06,.26,skin,side*.13,.03,.08,.03);}
 if(kind==='imp'){for(const side of [-1,1]){const horn=cone(body,.04,.16,M.leather,side*.1,1.04,.02,5);horn.rotation.z=-side*.35;const shape=new THREE.Shape();shape.moveTo(0,0);shape.lineTo(side*.34,.2);shape.lineTo(side*.3,-.02);shape.lineTo(side*.18,.04);shape.lineTo(0,-.12);const wing=part(body,new THREE.ShapeGeometry(shape),M.wing,side*.12,.72,-.17);wings.push(wing);}const tail=cone(body,.03,.42,skin,0,.42,-.3,5);tail.rotation.x=-2.1;}
 if(kind==='mummy')for(let i=0;i<6;i++){const wrap=rounded(body,torsoW+.03,.03,.33,M.leather,0,.44+i*.075,0,.012);wrap.rotation.z=(i%2?1:-1)*.12;}
 if(kind==='orc'){for(const x of [-.09,.09]){const tusk=cone(body,.045,.15,M.whiteFur,x,.91,.19,5);tusk.rotation.x=x<0?.35:-.35;}for(const x of [-.31,.31])sphere(body,.16,M.darkSteel,x,.84,0,1,.75,1);}
 if(kind==='dwarf'){cylinder(body,.22,.25,.15,M.darkSteel,0,1.17,0,10);const beard=sphere(body,.2,M.beard,0,1.0,.18,.95,1.1,.6);beard.scale.y=1.25;}
 if(guard){cylinder(body,.23,.23,.13,M.darkSteel,0,1.19,0,10);const plume=cone(body,.06,.25,M.redCloth,0,1.38,-.01,6);plume.rotation.z=-.12;rounded(body,.48,.07,.32,M.gold,0,.78,0,.02);}
 if(shopkeeper){rounded(body,.19,.26,.07,M.leather,.28,.67,.16,.025);const hat=cylinder(body,.25,.2,.13,M.brownCloth,0,1.2,0,12);hat.rotation.x=.04;}
 eyes(body,kind==='orc'||kind==='imp'?M.fire:undead?M.deadEye:M.eye,short?.91:1.04,.205,.075);
 if(guard){const spear=rounded(body,.045,.7,.045,M.steel,.36,.7,.24,.01);spear.rotation.z=-.12;cone(body,.07,.14,M.steel,.36,1.1,.24,5).rotation.x=Math.PI;}
 if(kind==='dwarf'){const pick=rounded(body,.045,.55,.045,M.steel,-.38,.67,.18,.01);pick.rotation.z=.55;const head=rounded(body,.26,.05,.05,M.steel,-.38,.94,.18,.01);head.rotation.z=-.2;}
 return actor(g,body,legs,null,wings,kind);
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
// ---- Class-based bestiary: every common monster letter gets its own silhouette ----

const cache=new Map();
function mat(color,options={}){const key=color+JSON.stringify(options);if(!cache.has(key))cache.set(key,new THREE.MeshStandardMaterial({color,roughness:.88,...options}));return cache.get(key);}
// NetHack's 16 terminal colours, pulled toward natural pigments so tints don't look neon.
const NH_COLORS=['#34343c','#a83b2e','#4f8a3a','#8a6440','#3d5fb0','#8a3f8f','#3f9a9a','#8f8f88',null,'#d9782e','#7fbf4f','#d6ac3a','#5f8fe0','#b85cbf','#6fd0d0','#e2ded2'];
function nhColor(cell){return Number.isInteger(cell.color)?NH_COLORS[cell.color]??null:null;}
function shade(hex,k){return '#'+new THREE.Color(hex).multiplyScalar(k).getHexString();}
const nose=mat('#1b1716',{roughness:.5}),darkEye=mat('#0e0c0b',{roughness:.2,metalness:.2});

function tube(parent,points,radius,material,segments=16){return part(parent,new THREE.TubeGeometry(new THREE.CatmullRomCurve3(points.map(p=>new THREE.Vector3(...p))),segments,radius,8,false),material);}

// Jackals, coyotes, foxes, wolves: lean body, tall ears, long muzzle, bushy tail.
function canine(o){
 const g=new THREE.Group(),body=new THREE.Group(),legs=[];g.add(body);g.scale.setScalar(o.scale||1);
 const coat=mat(o.coat),back=mat(o.back||shade(o.coat,.55)),belly=mat(o.belly||shade(o.coat,1.45)),legH=o.legH||.3,y=legH+.1;
 sphere(body,.2,coat,0,y,-.03,.72,.7,1.5);sphere(body,.17,coat,0,y+.03,.16,.82,.9,.9);sphere(body,.12,belly,0,y-.07,.14,.75,.6,1.1);sphere(body,.15,back,0,y+.1,-.08,.72,.38,1.3);
 const neck=cylinder(body,.07,.1,.22,coat,0,y+.15,.26,8);neck.rotation.x=.8;
 const head=new THREE.Group();head.position.set(0,y+.26,.34);body.add(head);
 sphere(head,.1,coat,0,0,0,.95,.85,1.05);
 const snoutL=o.snout||.2,snout=cylinder(head,.03,.065,snoutL,coat,0,-.035,.06+snoutL/2,10);snout.rotation.x=Math.PI/2;
 sphere(head,.05,belly,0,-.07,.1,.9,.5,1.5);sphere(head,.03,nose,0,-.03,.06+snoutL,1,.85,1);
 for(const side of [-1,1]){const ear=cone(head,.045,o.ears||.15,coat,side*.055,.11,-.02,4);ear.rotation.z=-side*.28;const inner=cone(head,.025,(o.ears||.15)*.7,belly,side*.055,.1,.0,4);inner.rotation.z=-side*.28;sphere(head,.018,darkEye,side*.05,.025,.08);}
 if(o.horns)for(const side of [-1,1]){const horn=cone(head,.03,.14,mat('#d8cfb8'),side*.08,.1,.02,5);horn.rotation.z=-side*.7;}
 for(const x of [-.085,.085])for(const z of [-.2,.17]){const leg=new THREE.Group();leg.position.set(x,y-.03,z);body.add(leg);rounded(leg,.06,legH,.065,coat,0,-legH/2,0,.02);sphere(leg,.035,o.socks?mat(o.socks):back,0,-legH+.02,.02,1,.7,1.3);legs.push(leg);}
 const tail=new THREE.Group();tail.position.set(0,y+.05,-.28);body.add(tail);
 // bushy tail: a lathe profile swept along a drooping curve, dark (or white) tip
 const tailCurve=new THREE.CatmullRomCurve3([[0,0,0],[0,-.02,-.1],[0,-.1,-.18],[0,-.22,-.23],[0,-.32,-.24]].map(p=>new THREE.Vector3(...p)));
 const bush=o.bushy??.04,samples=12;
 for(let i=0;i<samples;i++){const t=i/(samples-1),p=tailCurve.getPoint(t),r=.03+Math.sin(Math.min(1,t*1.15)*Math.PI*.95)*bush+(t>.8?-.02*(t-.8)/.2:0);
  const seg=sphere(tail,Math.max(.015,r),t>.78&&o.tip?mat(o.tip):coat,p.x,p.y,p.z);const tan=tailCurve.getTangent(t);seg.quaternion.setFromUnitVectors(new THREE.Vector3(0,1,0),tan);seg.scale.set(1,1.6,1);}
 return actor(g,body,legs,tail,[],o.quirk||'canine');
}
const CANINES={
 jackal:{coat:'#b98b55',back:'#3a3430',belly:'#e4d2ab',tip:'#2a2522',ears:.19,snout:.22,legH:.31},
 werejackal:{coat:'#8a6a4a',back:'#2a2420',belly:'#b9a58a',tip:'#1f1a18',ears:.19,snout:.22,legH:.31},
 coyote:{coat:'#94806a',back:'#5e5246',belly:'#dccfb8',tip:'#2c2825',ears:.17,snout:.21,scale:1.08},
 fox:{coat:'#c9652b',back:'#b0531f',belly:'#f1e7d8',tip:'#f5f0e8',socks:'#1e1a18',ears:.15,snout:.18,legH:.22,scale:.85,bushy:.06},
 wolf:{coat:'#8a8a86',back:'#4f4f4d',belly:'#d5d3cc',tip:'#2b2b2b',ears:.14,snout:.2,legH:.34,scale:1.2,bushy:.05},
 warg:{coat:'#5a524a',back:'#2c2825',belly:'#8a8176',tip:'#1f1c1a',ears:.13,snout:.2,legH:.36,scale:1.4,bushy:.05},
 'hell hound':{coat:'#7a2a20',back:'#2a100c',belly:'#c2562c',tip:'#ff7a2a',ears:.14,snout:.2,legH:.34,scale:1.3},
 rothe:{coat:'#6a4e36',back:'#3e2c1e',belly:'#8d7258',ears:.06,snout:.12,legH:.24,scale:1.1,horns:true,bushy:.01},
};

function feline(o){
 const g=new THREE.Group(),body=new THREE.Group(),legs=[];g.add(body);g.scale.setScalar(o.scale||1);
 const coat=mat(o.coat),dark=mat(o.stripe||shade(o.coat,.5)),light=mat(shade(o.coat,1.4)),legH=o.legH||.22,y=legH+.1;
 sphere(body,.18,coat,0,y,0,.75,.72,1.55);sphere(body,.11,light,0,y-.07,.1,.7,.55,1.1);
 for(let i=0;i<(o.stripes??3);i++)rounded(body,.27,.02,.035,dark,0,y+.1,-.12+i*.1,.01);
 const head=new THREE.Group();head.position.set(0,y+.13,.29);body.add(head);
 sphere(head,.11,coat,0,0,0,1.05,.92,.95);sphere(head,.05,light,0,-.035,.085,1.2,.7,.8);sphere(head,.016,nose,0,-.01,.12);
 for(const side of [-1,1]){const ear=cone(head,.045,.1,coat,side*.065,.1,-.01,3);ear.rotation.z=-side*.2;sphere(head,.02,mat(o.eye||'#d6b640',{emissive:o.eye||'#6a5010',emissiveIntensity:.6}),side*.045,.02,.09,1,.8,.6);}
 for(const x of [-.08,.08])for(const z of [-.18,.16]){const leg=new THREE.Group();leg.position.set(x,y-.03,z);body.add(leg);rounded(leg,.055,legH,.06,coat,0,-legH/2,0,.02);legs.push(leg);}
 const tail=new THREE.Group();tail.position.set(0,y+.05,-.26);body.add(tail);tube(tail,[[0,0,0],[0,.1,-.12],[.04,.28,-.16],[.08,.4,-.1]],.028,coat);
 return actor(g,body,legs,tail,[],'feline');
}
const FELINES={kitten:{coat:'#c98f55',scale:.7},housecat:{coat:'#c98f55'},'large cat':{coat:'#c98f55',scale:1.15},jaguar:{coat:'#c79a45',stripe:'#3a2a18',scale:1.4,stripes:5},lynx:{coat:'#a88f70',scale:1.1,ears:.14},panther:{coat:'#26242a',stripe:'#1a181c',scale:1.45,eye:'#9ad04a'},tiger:{coat:'#d17a2a',stripe:'#1e1510',scale:1.6,stripes:6},'displacer beast':{coat:'#3a3450',scale:1.5}};

// Newts, geckos, iguanas, lizards, crocodiles: low splayed body and a long tapering tail.
function lizard(o){
 const g=new THREE.Group(),body=new THREE.Group(),legs=[];g.add(body);g.scale.setScalar(o.scale||1);
 const skin=mat(o.skin),belly=mat(o.belly||shade(o.skin,1.3)),spot=mat(o.spot||shade(o.skin,.45));
 sphere(body,.12,skin,0,.13,0,.95,.5,1.9);sphere(body,.08,belly,0,.09,.02,.9,.35,1.7);
 const head=new THREE.Group();head.position.set(0,.14,.27);body.add(head);
 rounded(head,.15,.07,.17,skin,0,0,.03,.03);for(const side of [-1,1])sphere(head,.028,darkEye,side*.065,.04,.02);
 for(let i=0;i<4;i++)sphere(body,.03,spot,((i*37)%3-1)*.04,.19,-.12+i*.08,1,.35,1.2);
 for(const side of [-1,1])for(const z of [-.12,.13]){const leg=new THREE.Group();leg.position.set(side*.1,.13,z);body.add(leg);const upper=rounded(leg,.13,.035,.04,skin,side*.07,-.03,0,.012);upper.rotation.z=side*-.5;rounded(leg,.05,.02,.07,skin,side*.13,-.1,.02,.008);legs.push(leg);}
 const tail=new THREE.Group();tail.position.set(0,.13,-.2);body.add(tail);
 let px=0,pz=0;for(let i=0;i<6;i++){const r=.055*(1-i/7),len=.09;const seg=cylinder(tail,r*.8,r,len,skin,px,-.012*i,pz-len/2,8);seg.rotation.x=Math.PI/2;px+=Math.sin(i*.6)*.012;pz-=len*.95;}
 return actor(g,body,legs,tail,[],'lizard');
}
const LIZARDS={newt:{skin:'#d69a38',belly:'#e9763a',spot:'#5a3a1a',scale:.8},gecko:{skin:'#6f9a45',scale:.8},iguana:{skin:'#7a6a42',scale:1},'baby crocodile':{skin:'#5f6a3a',scale:1},lizard:{skin:'#4f8a3a',scale:1},chameleon:{skin:'#6aa08a',scale:1},crocodile:{skin:'#4f5a32',scale:1.6},salamander:{skin:'#d9582a',belly:'#ffb040',scale:1.4}};

// Lichens and molds: stationary crusts and mounds. Mushrooms for shriekers and violet fungi.
function fungus(o){
 const g=new THREE.Group(),body=new THREE.Group();g.add(body);
 const main=mat(o.color,{roughness:.95}),dark=mat(shade(o.color,.55)),bright=mat(shade(o.color,1.35),{emissive:shade(o.color,.35),emissiveIntensity:.35});
 if(o.form==='lichen'){
  for(let i=0;i<9;i++){const a=i*2.4,r=i?.08+((i*53)%10)/60:0;sphere(body,.09+((i*29)%5)/60,i%3?main:dark,Math.cos(a)*r,.05,Math.sin(a)*r,1,.35,1);}
  for(let i=0;i<5;i++){const a=i*1.3+.4;const cup=cylinder(body,.045,.015,.12,bright,Math.cos(a)*.15,.1,Math.sin(a)*.15,8);cup.rotation.z=Math.cos(a)*.3;}
 } else if(o.form==='mushroom'){
  cylinder(body,.07,.1,.42,mat('#d8cdb5'),0,.21,0,10);
  part(body,new THREE.SphereGeometry(.26,18,10,0,Math.PI*2,0,Math.PI/2),main,0,.4,0).scale.y=.7;
  for(let i=0;i<7;i++){const a=i*.9;sphere(body,.035,mat('#efe6d0'),Math.cos(a)*.15,.52,Math.sin(a)*.15,1,.5,1);}
  if(o.tendrils)for(let i=0;i<4;i++){const a=i*Math.PI/2+.4;tube(body,[[Math.cos(a)*.2,.36,Math.sin(a)*.2],[Math.cos(a)*.34,.2,Math.sin(a)*.34],[Math.cos(a)*.3,.02,Math.sin(a)*.3]],.018,dark,10);}
 } else {
  sphere(body,.24,main,0,.12,0,1,.62,1);
  for(let i=0;i<11;i++){const a=i*2.1,r=.1+(i%3)*.05;sphere(body,.05+(i%4)*.018,i%2?bright:dark,Math.cos(a)*r,.18+((i*7)%3)*.03,Math.sin(a)*r);}
  for(let i=0;i<6;i++){const a=i*1.05;sphere(body,.016,bright,Math.cos(a)*.16,.32+(i%2)*.05,Math.sin(a)*.16);}
 }
 return actor(g,body,[],null,[],'fungus');
}

// Blobs, jellies, puddings: translucent mass with a visible nucleus.
function blob(o){
 const g=new THREE.Group(),body=new THREE.Group();g.add(body);
 const skin=new THREE.MeshStandardMaterial({color:o.color,emissive:o.color,emissiveIntensity:.18,roughness:.15,transparent:true,opacity:.78});
 sphere(body,.28,skin,0,o.flat?.12:.2,0,1,o.flat?.45:.72,1);sphere(body,.09,mat(shade(o.color,.4)),0,o.flat?.12:.2,0,1,.8,1);
 if(o.flat)for(let i=0;i<6;i++){const a=i*Math.PI/3;tube(body,[[Math.cos(a)*.2,.08,Math.sin(a)*.2],[Math.cos(a)*.34,.03,Math.sin(a)*.34],[Math.cos(a)*.4,.01,Math.sin(a)*.4]],.022,skin,8);}
 else for(let i=0;i<5;i++){const a=i*1.3;sphere(body,.07,skin,Math.cos(a)*.24,.07,Math.sin(a)*.24,1,.6,1);}
 return actor(g,body,[],null,[],'blob');
}

// Floating eyes: a big eyeball hovering at head height. Easily the most recognisable shape.
function floatingEye(o){
 const g=new THREE.Group(),body=new THREE.Group(),lift=new THREE.Group();g.add(body);body.add(lift);lift.position.y=.58;
 sphere(lift,.24,mat('#ebe6da',{roughness:.3}));
 sphere(lift,.115,mat(o.iris||'#2f6ad0',{roughness:.2,emissive:o.iris||'#2f6ad0',emissiveIntensity:.25}),0,0,.2,1,1,.38);
 sphere(lift,.05,mat('#050505',{roughness:.1}),0,0,.24,1,1,.35);
 for(let i=0;i<6;i++){const a=i*1.05;tube(lift,[[Math.cos(a)*.12,-.18,Math.sin(a)*.12],[Math.cos(a)*.16,-.32,Math.sin(a)*.16],[Math.cos(a)*.12,-.44,Math.sin(a)*.12]],.012,mat('#b98a7a'),8);}
 for(let i=0;i<5;i++){const a=i*1.3-2.6;const vein=rounded(lift,.006,.12,.006,mat('#b8453a'),Math.sin(a)*.2,Math.cos(a)*.08,.1,.002);vein.rotation.z=a;}
 return actor(g,body,[],null,[],'hover');
}

// Yellow/black lights: a glowing mote. Explodes when it touches you, so it should glow.
function wisp(o){
 const g=new THREE.Group(),body=new THREE.Group(),lift=new THREE.Group();g.add(body);body.add(lift);lift.position.y=.55;
 const core=sphere(lift,.12,new THREE.MeshStandardMaterial({color:o.color,emissive:o.color,emissiveIntensity:5,roughness:.2}));
 sphere(lift,.24,new THREE.MeshStandardMaterial({color:o.color,emissive:o.color,emissiveIntensity:1.2,transparent:true,opacity:.28,depthWrite:false}));
 for(let i=0;i<6;i++){const a=i*1.05;sphere(lift,.03,core.material,Math.cos(a)*.3,Math.sin(a*2)*.08,Math.sin(a)*.3);}
 g.userData.core=core;return Object.assign(actor(g,body,[],null,[],'hover'),{core});
}

// Ants, bees: three body segments and six legs; bees add striped abdomen and wings.
function insect(o){
 const g=new THREE.Group(),body=new THREE.Group(),legs=[],wings=[];g.add(body);g.scale.setScalar(o.scale||1);
 const shell=mat(o.color,{roughness:.45,metalness:.1}),dark=mat('#15130f',{roughness:.4}),y=o.fly?.5:.2;
 sphere(body,.075,o.bee?dark:shell,0,y+.02,.2);sphere(body,.085,o.bee?mat('#5a4020'):shell,0,y,.06,1,.9,1.1);
 const abdomen=sphere(body,.13,shell,0,y+.03,-.15,.9,.85,1.35);abdomen.rotation.x=o.bee?.3:-.25;
 if(o.bee){for(let i=0;i<3;i++)rounded(body,.23-i*.03,.2-i*.03,.035,dark,0,y+.03-i*.03,-.1-i*.07,.08);cone(body,.02,.1,dark,0,y-.04,-.33,5).rotation.x=-2.2;
  for(const side of [-1,1]){const shape=new THREE.Shape();shape.moveTo(0,0);shape.quadraticCurveTo(side*.2,.18,side*.32,.04);shape.quadraticCurveTo(side*.18,-.04,0,0);const wing=part(body,new THREE.ShapeGeometry(shape),mat('#dfeaf0',{transparent:true,opacity:.45,side:THREE.DoubleSide,depthWrite:false}),side*.03,y+.1,.03);wing.rotation.x=-Math.PI/2+.25;wings.push(wing);}}
 for(let i=0;i<2;i++){const side=i?1:-1;tube(body,[[side*.03,y+.07,.26],[side*.09,y+.18,.3],[side*.14,y+.2,.38]],.009,dark,8);}
 for(const side of [-1,1])for(const z of [-.02,.06,.14]){const leg=new THREE.Group();leg.position.set(side*.06,y-.02,z);body.add(leg);tube(leg,[[0,0,0],[side*.12,.07,(z-.06)*.6],o.fly?[side*.16,-.12,(z-.06)*1.2-.04]:[side*.22,-y+.03,(z-.06)*1.6]],.011,dark,8);legs.push(leg);}
 return actor(g,body,legs,null,wings,o.fly?'bee':'insect');
}
const INSECTS={'giant ant':{color:'#6a3f22'},'killer bee':{color:'#d8a92a',bee:true,fly:true,scale:.75},'soldier ant':{color:'#34457a',scale:1.15},'fire ant':{color:'#b03a22'},'giant beetle':{color:'#222028',scale:1.5},'queen bee':{color:'#b98a2a',bee:true,fly:true,scale:1.1}};

function spider(o){
 const g=new THREE.Group(),body=new THREE.Group(),legs=[];g.add(body);g.scale.setScalar(o.scale||1);
 const shell=mat(o.color,{roughness:.6}),dark=mat(shade(o.color,.45)),y=.24;
 sphere(body,.1,shell,0,y,.08,1,.8,1.05);sphere(body,.16,shell,0,y+.05,-.14,1,.85,1.2);
 rounded(body,.12,.02,.16,dark,0,y+.19,-.14,.01);
 for(const side of [-1,1])for(const [dx,dy] of [[.03,.02],[.06,.0]])sphere(body,.018,mat('#c81e1e',{emissive:'#ff2a1a',emissiveIntensity:1.5}),side*dx,y+.04+dy,.17);
 for(const side of [-1,1])for(let i=0;i<4;i++){const z=.14-i*.06,spread=(i-1.5)*.35;const leg=new THREE.Group();leg.position.set(side*.07,y,z);body.add(leg);tube(leg,[[0,0,0],[side*.16,.16,Math.sin(spread)*.12],[side*.3,-y+.01,Math.sin(spread)*.3]],.014,dark,10);legs.push(leg);}
 return actor(g,body,legs,null,[],'spider');
}
function centipede(o){
 const g=new THREE.Group(),body=new THREE.Group(),legs=[];g.add(body);
 const shell=mat(o.color,{roughness:.5}),dark=mat(shade(o.color,.4));
 for(let i=0;i<8;i++){const z=.3-i*.085,x=Math.sin(i*.7)*.05;sphere(body,.055-(i>5?(i-5)*.008:0),i%2?shell:mat(shade(o.color,.8)),x,.09,z,1.1,.7,1);for(const side of [-1,1]){const leg=new THREE.Group();leg.position.set(x+side*.04,.09,z);body.add(leg);tube(leg,[[0,0,0],[side*.08,.03,0],[side*.13,-.08,.02]],.008,dark,6);legs.push(leg);}}
 for(const side of [-1,1])tube(body,[[side*.02,.12,.34],[side*.08,.2,.44],[side*.14,.2,.5]],.007,dark,6);
 return actor(g,body,legs,null,[],'insect');
}

// Bats: big scalloped wings and ears. Wings flap in live.js via the 'bat' quirk.
function bat(o){
 const g=new THREE.Group(),body=new THREE.Group(),lift=new THREE.Group(),wings=[];g.add(body);body.add(lift);lift.position.y=.62;g.scale.setScalar(o.scale||1);
 const fur=mat(o.color),membrane=mat(shade(o.color,.6),{side:THREE.DoubleSide,roughness:.8});
 sphere(lift,.085,fur,0,0,0,1,1.15,.9);sphere(lift,.065,fur,0,.08,.05);
 for(const side of [-1,1]){const ear=cone(lift,.03,.1,fur,side*.035,.16,.04,4);ear.rotation.z=-side*.3;sphere(lift,.012,mat('#ff5a3a',{emissive:'#ff3a1a',emissiveIntensity:2}),side*.025,.09,.105);
  const shape=new THREE.Shape();shape.moveTo(0,.05);shape.lineTo(side*.2,.14);shape.lineTo(side*.42,.08);shape.quadraticCurveTo(side*.36,-.02,side*.3,-.08);shape.quadraticCurveTo(side*.22,-.02,side*.16,-.1);shape.quadraticCurveTo(side*.08,-.04,0,-.06);
  const pivot=new THREE.Group();pivot.position.set(side*.05,.02,0);lift.add(pivot);part(pivot,new THREE.ShapeGeometry(shape),membrane);pivot.userData.side=side;wings.push(pivot);}
 return actor(g,body,[],null,wings,'bat');
}

function snake(o){
 const g=new THREE.Group(),body=new THREE.Group();g.add(body);g.scale.setScalar(o.scale||1);
 const skin=mat(o.color,{roughness:.55}),belly=mat(o.belly||shade(o.color,1.5)),coil=[];
 for(let i=0;i<=28;i++){const t=i/28,a=t*Math.PI*3.6,r=.24-t*.13;coil.push([Math.cos(a)*r,.05+t*.1,Math.sin(a)*r]);}
 coil.push([0,.3,.06],[0,.42,.14]);
 tube(body,coil,.045,skin,64);
 const head=new THREE.Group();head.position.set(0,.44,.18);body.add(head);
 sphere(head,.065,skin,0,0,0,1,.6,1.35);sphere(head,.04,belly,0,-.02,.03,1,.4,1.3);
 if(o.hood)sphere(head,.13,skin,0,-.08,-.06,1.2,1,.25);
 for(const side of [-1,1])sphere(head,.016,mat('#e0b020',{emissive:'#6a4a00',emissiveIntensity:.8}),side*.04,.02,.05);
 const tongue=rounded(head,.012,.004,.12,mat('#c0282a'),0,-.01,.12,.002);tongue.rotation.x=.2;
 return actor(g,body,[],null,[],'snake');
}
const SNAKES={'garter snake':{color:'#3f7a34',belly:'#d6c84a',scale:.75},snake:{color:'#7a5a34'},'water moccasin':{color:'#5a3228'},'pit viper':{color:'#3a5a8a'},python:{color:'#7a5a7a',scale:1.4},cobra:{color:'#3a4a7a',hood:true}};

// Generic guardian, kept as the last resort but tinted by the monster's glyph colour.
function guardian(o={}){const g=new THREE.Group(),body=new THREE.Group();g.add(body);const armor=o.color?mat(shade(o.color,.7),{roughness:.5,metalness:.4}):M.darkSteel;rounded(body,.42,.78,.38,armor,0,.5,0,.07);sphere(body,.23,M.graySkin,0,1.03,0,1,.9,1);for(const x of [-.4,.4])rounded(body,.25,.5,.3,o.color?mat(o.color,{roughness:.4,metalness:.3}):M.steel,x,.58,0,.05);const core=sphere(body,.09,M.fire,0,.62,.23);g.userData.core=core;eyes(body,M.fire,1.04,.22,.08);return Object.assign(actor(g,body),{core});}

const SKIN={kobold:'#8a5a3a','large kobold':'#9a3f2f','kobold lord':'#7a3f70','kobold shaman':'#5070a8',homunculus:'#5f8a3f',imp:'#a53a2a',manes:'#8a2f2a',lemure:'#6a5040',quasit:'#3f5fa0',tengu:'#3f9a9a'};
const ZOMBIE_SKIN={'kobold zombie':'#7a6a48','gnome zombie':'#7d6b55','orc zombie':'#5f6f4d','dwarf zombie':'#7a5a4a','elf zombie':'#8a9a7a','human zombie':'#a3a792','ettin zombie':'#6a6f80','giant zombie':'#7a7a6a'};

export function createCreature(cell={}){
 const name=(cell.name||'').toLowerCase(),letter=Number.isInteger(cell.symbol)?String.fromCharCode(cell.symbol):'',color=nhColor(cell);
 if(/^(sewer rat|giant rat|rabid rat|rat)$/.test(name))return rat(name==='giant rat');
 if(/grid ?bug/.test(name))return gridBug();
 if(CANINES[name])return canine(CANINES[name]);
 if(/^(little dog|dog|large dog)$/.test(name))return dog();
 if(FELINES[name])return feline(FELINES[name]);
 if(LIZARDS[name])return lizard(LIZARDS[name]);
 if(INSECTS[name])return insect(INSECTS[name]);
 if(SNAKES[name])return snake(SNAKES[name]);
 if(name==='floating eye')return floatingEye({});
 if(/ light$/.test(name))return wisp({color:color||(name.startsWith('black')?'#4a2a8a':'#ffd23a')});
 if(name==='lichen')return fungus({form:'lichen',color:'#8fbf5a'});
 if(/mold$/.test(name))return fungus({form:'mound',color:color||{yellow:'#d6b43c',green:'#5fa044',brown:'#8a6440',red:'#b8402e'}[name.split(' ')[0]]||'#8a8a60'});
 if(name==='shrieker'||name==='violet fungus')return fungus({form:'mushroom',color:name==='shrieker'?'#8f5aa8':'#b05ac0',tendrils:name==='violet fungus'});
 if(name==='cave spider'||name==='giant spider')return spider({color:name==='cave spider'?'#7a7a74':'#4a2a5a',scale:name==='cave spider'?.65:1.5});
 if(/(blob|jelly|pudding|ooze|slime)$/.test(name))return blob({color:color||{acid:'#6fae3a','blue':'#3d6fd0','spotted':'#7a8a3a','ochre':'#c08a3a','brown':'#7a5a3a','black':'#2a2a30','gray':'#7a7a78','green':'#4f9a3a','quivering':'#b0a8d0','gelatinous':'#8ad0c0'}[name.split(' ')[0]]||'#7a9a6a',flat:/jelly$/.test(name)});
 if(name==='centipede')return centipede({color:'#c9a03a'});
 if(/^(bat|giant bat|vampire bat)$/.test(name))return bat({color:name==='bat'?'#5a4636':name==='giant bat'?'#7a3a32':'#28242a',scale:name==='giant bat'?1.25:1});
 if(ZOMBIE_SKIN[name])return humanoid('zombie',{skin:mat(ZOMBIE_SKIN[name]),cloth:mat('#3f3a34')});
 if(/mummy$/.test(name))return humanoid('mummy',{skin:mat('#6a5f4a'),cloth:mat('#c9bb98')});
 if(/shopkeeper|merchant/.test(name))return humanoid('shopkeeper');
 if(/guard|soldier|watchman|watch captain/.test(name))return humanoid('guard');
 if(/unicorn/.test(name))return unicorn();
 if(/dragon/.test(name))return dragon();
 if(SKIN[name])return humanoid(letter==='k'||/kobold/.test(name)?'kobold':'imp',{skin:mat(SKIN[name]),cloth:mat(shade(SKIN[name],.55))});
 if(name==='hobbit')return humanoid('hobbit',{cloth:mat('#4f7a3a')});
 if(/orc|uruk|snaga/.test(name))return humanoid('orc',color?{cloth:mat(shade(color,.75))}:{});
 if(/dwarf/.test(name))return humanoid('dwarf');
 if(/gnome/.test(name))return humanoid('gnome',color?{cap:mat(color)}:{});
 // unlisted species: fall back on the monster class letter, then the glyph colour
 const c=color||'#8a8a80';
 switch(letter){
  case 'd':return canine({coat:c,ears:.15,snout:.2});
  case 'f':return feline({coat:c});
  case ':':return lizard({skin:c});
  case 'a':return insect({color:c});
  case 's':return spider({color:c});
  case 'S':return snake({color:c});
  case 'B':return bat({color:c});
  case 'F':return fungus({form:'mound',color:c});
  case 'b':case 'j':case 'P':return blob({color:c,flat:letter==='j'});
  case 'e':return floatingEye({iris:c});
  case 'y':return wisp({color:c});
  case 'k':return humanoid('kobold',{skin:mat(c),cloth:mat(shade(c,.55))});
  case 'i':return humanoid('imp',{skin:mat(c),cloth:mat(shade(c,.55))});
  case 'Z':return humanoid('zombie',{skin:mat(c),cloth:mat('#3f3a34')});
  case 'M':return humanoid('mummy',{skin:mat('#6a5f4a'),cloth:mat('#c9bb98')});
  case 'G':return humanoid('gnome',{cap:mat(c)});
  case 'h':return humanoid('dwarf');
  case 'o':return humanoid('orc',{cloth:mat(shade(c,.75))});
  case 'q':return canine({...CANINES.rothe,coat:c});
  case 'u':return unicorn();
  case 'D':return dragon();
  case '@':return humanoid('human',{cloth:mat(shade(c,.8))});
  case 'r':return rat(false);
  case 'x':return gridBug();
 }
 return guardian({color});
}
