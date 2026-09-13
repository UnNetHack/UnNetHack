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
 }else{
  // A restrained proxy for weapon families whose detailed models are still pending.
  part(new THREE.CylinderGeometry(.027,.035,.65,8),leather,0,.18);
  if(/axe/.test(name))part(new THREE.BoxGeometry(.3,.19,.045),steel,.08,.48);
  else if(/mace|hammer|club/.test(name))part(new THREE.BoxGeometry(.19,.18,.16),steel,0,.48);
  else if(/spear|pike|javelin/.test(name))part(new THREE.ConeGeometry(.065,.24,4),steel,0,.61);
 }
 g.userData.dispose=()=>{g.traverse(o=>o.geometry?.dispose());steel.dispose();leather.dispose();brass.dispose();};return g;
}
