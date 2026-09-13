import * as THREE from 'three';
import {RoundedBoxGeometry} from 'three/addons/geometries/RoundedBoxGeometry.js';
import {createFire} from './fire.js';

// A compact, walkable terrain feature; all ornaments stay inside its tile.
export function createAltar(){
 const g=new THREE.Group();g.name='Altar of the Last Ember';
 const stone=new THREE.MeshStandardMaterial({color:0x353943,roughness:.95});
 const edge=new THREE.MeshStandardMaterial({color:0x62616b,roughness:.86});
 const brass=new THREE.MeshStandardMaterial({color:0xb79455,metalness:.75,roughness:.46});
 const cloth=new THREE.MeshStandardMaterial({color:0x652b39,roughness:1});
 const wax=new THREE.MeshStandardMaterial({color:0xdac49a,roughness:.9});
 const ember=new THREE.MeshStandardMaterial({color:0xffd08b,emissive:0xff8c32,emissiveIntensity:3});
 function mesh(geo,mat,x,y,z){const m=new THREE.Mesh(geo,mat);m.position.set(x,y,z);m.castShadow=m.receiveShadow=true;g.add(m);return m;}
 function box(w,h,d,mat,x,y,z){return mesh(new RoundedBoxGeometry(w,h,d,2,.015),mat,x,y,z);}
 box(.9,.09,.8,edge,0,.045,0);box(.76,.08,.66,stone,0,.13,0);
 for(const x of [-.25,.25])box(.16,.36,.4,stone,x,.32,0);
 box(.84,.13,.65,edge,0,.53,0);box(.79,.025,.6,stone,0,.606,0);
 box(.29,.014,.58,cloth,0,.627,.03);box(.29,.24,.018,cloth,0,.51,.332);
 for(const x of [-.125,.125])box(.013,.22,.008,brass,x,.51,.345);
 for(let i=0;i<7;i++)box(.015,.055,.015,brass,(i-3)*.039,.366,.34);
 const seal=mesh(new THREE.TorusGeometry(.065,.009,5,24),brass,0,.52,.35);
 box(.012,.09,.008,brass,0,.52,.351);box(.07,.012,.008,brass,0,.52,.351);
 mesh(new THREE.CylinderGeometry(.115,.07,.065,12),brass,0,.666,.03);
 mesh(new THREE.CylinderGeometry(.099,.099,.006,12),stone,0,.702,.03);
 for(let i=0;i<5;i++)mesh(new THREE.DodecahedronGeometry(.016),i%2?ember:stone,Math.sin(i*2)*.06,.713,.03+Math.cos(i*2)*.055);
 for(const [x,z,h] of [[-.29,-.14,.19],[-.2,-.21,.12],[.28,-.15,.25],[.21,-.23,.16]]){
  mesh(new THREE.CylinderGeometry(.045,.055,.014,10),brass,x,.633,z);
  mesh(new THREE.CylinderGeometry(.027,.034,h,10),wax,x,.64+h/2,z);
  for(let i=0;i<3;i++)box(.012,h*(.25+i*.13),.014,wax,x+Math.sin(i*2)*.026,.64+h*.65,z+Math.cos(i*2)*.026);
  const flame=createFire(h*13);flame.position.set(x,.64+h,z);flame.scale.setScalar(.22);g.add(flame);
 }
 // Deliberately uneven mineral seams across the front edge.
 for(let i=0;i<5;i++){const seam=box(.007,.08,.004,stone,-.34+i*.15,.54,.327);seam.rotation.z=(i%2?1:-1)*.4;}
 g.userData.dispose=()=>{g.traverse(o=>o.geometry?.dispose());for(const m of [stone,edge,brass,cloth,wax,ember])m.dispose();};
 return g;
}
