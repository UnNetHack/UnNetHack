import * as THREE from 'three';

export function createHeldWeapon(item){
 const g=new THREE.Group();if(!item)return g;
 g.name=item.name;
 const steel=new THREE.MeshStandardMaterial({color:0xd0dce2,metalness:.8,roughness:.23});
 const leather=new THREE.MeshStandardMaterial({color:0x442c22,roughness:.92});
 const brass=new THREE.MeshStandardMaterial({color:0xbe9650,metalness:.7,roughness:.35});
 function part(geometry,material,x,y,z=0){const m=new THREE.Mesh(geometry,material);m.position.set(x,y,z);m.castShadow=true;g.add(m);return m;}
 const name=(item.name||'').toLowerCase();
 const blade=/sword|dagger|knife|athame|saber|scimitar|katana|tsurugi|wakizashi/.test(name);
 if(blade){
  const short=/dagger|knife|athame/.test(name),length=short?.34:.75,width=short?.055:.075;
  part(new THREE.CylinderGeometry(.029,.035,.17,8),leather,0,0);
  part(new THREE.SphereGeometry(.044,8,6),brass,0,-.11);
  part(new THREE.BoxGeometry(short?.18:.27,.035,.065),brass,0,.105);
  // Diamond cross-section: bright bevels and a continuous pointed tip.
  const vertices=[-width,.13,0,0,.13,.024,width,.13,0,0,.13,-.024,-width*.65,length,0,0,length,.017,width*.65,length,0,0,length,-.017,0,length+.16,0];
  const geo=new THREE.BufferGeometry();geo.setAttribute('position',new THREE.Float32BufferAttribute(vertices,3));geo.setIndex([0,4,5,0,5,1,1,5,6,1,6,2,2,6,7,2,7,3,3,7,4,3,4,0,4,8,5,5,8,6,6,8,7,7,8,4,0,1,2,0,2,3]);geo.computeVertexNormals();part(geo,steel,0,0);
 }else if(/\bmace\b/.test(name)){
  // Flanged head and bound grip distinguish a mace from a square hammer.
  part(new THREE.CylinderGeometry(.024,.03,.57,10),steel,0,.18);
  part(new THREE.CylinderGeometry(.036,.036,.19,10),leather,0,-.015);
  for(let i=0;i<5;i++)part(new THREE.CylinderGeometry(.038,.038,.009,10),brass,0,-.09+i*.036);
  part(new THREE.SphereGeometry(.047,10,8),brass,0,-.135);
  part(new THREE.CylinderGeometry(.055,.065,.22,12),steel,0,.47);
  const outline=new THREE.Shape();outline.moveTo(.045,.35);outline.lineTo(.115,.39);outline.lineTo(.14,.51);outline.lineTo(.09,.585);outline.lineTo(.045,.58);outline.closePath();
  for(let i=0;i<6;i++){
   const flange=new THREE.ExtrudeGeometry(outline,{depth:.018,bevelEnabled:true,bevelThickness:.003,bevelSize:.004,bevelSegments:1,steps:1});flange.translate(0,0,-.009);
   part(flange,steel,0,0).rotation.y=i*Math.PI/3;
  }
  for(const y of [.35,.59])part(new THREE.CylinderGeometry(.068,.068,.022,12),brass,0,y);
 }else if(/\b(war hammer|hammer)\b/.test(name)){
  part(new THREE.CylinderGeometry(.025,.032,.65,10),leather,0,.18);
  for(let i=0;i<5;i++)part(new THREE.CylinderGeometry(.034,.034,.012,10),brass,0,-.09+i*.035);
  part(new THREE.SphereGeometry(.043,10,8),brass,0,-.15);
  // Forged transverse head: broad striking face, central eye, tapered rear peen.
  const head=new THREE.CylinderGeometry(.072,.085,.22,4);head.rotateZ(Math.PI/2);part(head,steel,-.045,.48);
  const face=new THREE.CylinderGeometry(.089,.089,.035,4);face.rotateZ(Math.PI/2);part(face,steel,-.17,.48);
  const peen=new THREE.ConeGeometry(.068,.18,4);peen.rotateZ(-Math.PI/2);part(peen,steel,.145,.48);
  part(new THREE.BoxGeometry(.06,.17,.12),brass,0,.48);
  part(new THREE.CylinderGeometry(.041,.041,.045,10),brass,0,.37);
 }else if(/\baxe\b/.test(name)){
  part(new THREE.CylinderGeometry(.026,.036,.68,10),leather,0,.18);
  for(let i=0;i<5;i++)part(new THREE.CylinderGeometry(.037,.037,.01,10),brass,0,-.12+i*.033);
  part(new THREE.CylinderGeometry(.045,.045,.12,10),steel,0,.47);
  const outline=new THREE.Shape();outline.moveTo(.02,.53);outline.quadraticCurveTo(.14,.56,.24,.63);outline.quadraticCurveTo(.29,.45,.23,.29);outline.quadraticCurveTo(.13,.39,.02,.4);outline.closePath();
  const axeBlade=new THREE.ExtrudeGeometry(outline,{depth:.025,bevelEnabled:true,bevelSize:.008,bevelThickness:.005,bevelSegments:2,steps:1,curveSegments:10});axeBlade.translate(0,0,-.0125);part(axeBlade,steel,0,0);
  part(new THREE.BoxGeometry(.09,.085,.065),steel,-.055,.47);
  if(/battle-axe/.test(name)){const second=axeBlade.clone();second.rotateY(Math.PI);part(second,steel,0,0);}
 }else if(/\bclub\b/.test(name)){
  // A carved wooden striking head flows into the grip, without a metal cube.
  const profile=[new THREE.Vector2(0,-.15),new THREE.Vector2(.038,-.14),new THREE.Vector2(.029,-.09),new THREE.Vector2(.028,.09),new THREE.Vector2(.047,.22),new THREE.Vector2(.078,.4),new THREE.Vector2(.086,.49),new THREE.Vector2(.058,.55),new THREE.Vector2(0,.57)];
  part(new THREE.LatheGeometry(profile,12),leather,0,0);
  for(let i=0;i<5;i++)part(new THREE.CylinderGeometry(.033,.033,.014,10),brass,0,-.08+i*.031);
  const grain=new THREE.MeshStandardMaterial({color:0x281b15,roughness:1});
  for(let i=0;i<7;i++){
   const a=i*Math.PI*2/7;
   const points=[[.21,.046],[.33,.064],[.44,.082],[.51,.075]].map(([y,r],j)=>new THREE.Vector3(Math.cos(a+j*.025)*r,y,Math.sin(a+j*.025)*r));
   part(new THREE.TubeGeometry(new THREE.CatmullRomCurve3(points),8,.002,3,false),grain,0,0);
  }
  g.userData.extraMaterial=grain;
 }else{
  // A restrained proxy for weapon families whose detailed models are still pending.
  part(new THREE.CylinderGeometry(.027,.035,.65,8),leather,0,.18);
  if(/axe/.test(name))part(new THREE.BoxGeometry(.3,.19,.045),steel,.08,.48);
  else if(/mace|hammer|club/.test(name))part(new THREE.BoxGeometry(.19,.18,.16),steel,0,.48);
  else if(/spear|pike|javelin/.test(name))part(new THREE.ConeGeometry(.065,.24,4),steel,0,.61);
 }
 g.userData.dispose=()=>{g.traverse(o=>o.geometry?.dispose());g.userData.extraMaterial?.dispose();steel.dispose();leather.dispose();brass.dispose();};return g;
}
