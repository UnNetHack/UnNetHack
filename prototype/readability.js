import * as THREE from 'three';

// Readability staging for creatures at gameplay zoom: an ink outline of constant screen
// width, a disposition ring on the floor, and a minimum on-screen size.

const outlineMaterial=new THREE.ShaderMaterial({
 uniforms:{color:{value:new THREE.Color(0x07090a)},width:{value:.0019}},
 vertexShader:`uniform float width;void main(){vec4 mv=modelViewMatrix*vec4(position,1.);vec3 n=normalize(normalMatrix*normal);mv.xyz+=n*width*-mv.z;gl_Position=projectionMatrix*mv;}`,
 fragmentShader:`uniform vec3 color;void main(){gl_FragColor=vec4(color,1.);}`,
 side:THREE.BackSide,
});

function ringMaterial(color){
 const c=document.createElement('canvas');c.width=c.height=128;const ctx=c.getContext('2d');
 const g=ctx.createRadialGradient(64,64,34,64,64,62);g.addColorStop(0,'rgba(255,255,255,0)');g.addColorStop(.55,'rgba(255,255,255,.95)');g.addColorStop(.75,'rgba(255,255,255,.55)');g.addColorStop(1,'rgba(255,255,255,0)');
 ctx.fillStyle=g;ctx.fillRect(0,0,128,128);
 const texture=new THREE.CanvasTexture(c);
 return new THREE.MeshBasicMaterial({map:texture,color,transparent:true,depthWrite:false,toneMapped:false,opacity:.85});
}
const RINGS={hostile:ringMaterial(0xff4a2a),pet:ringMaterial(0x4fe08a),peaceful:ringMaterial(0xe8c860)};
const ringGeo=new THREE.PlaneGeometry(1,1).rotateX(-Math.PI/2);

const MIN_SIZE=.72,MAX_FOOTPRINT=.98,MAX_SCALE=1.8;

// Idempotent: meshes added later (a newly wielded weapon) can be outlined by calling again.
export function addOutlines(g){
 g.traverse(mesh=>{
  if(!mesh.isMesh||mesh.userData.outline||mesh.userData.ring||mesh.userData.hasOutline)return;
  const geo=mesh.geometry;mesh.userData.hasOutline=true;
  if(geo.type==='ShapeGeometry'||geo.type==='PlaneGeometry'||mesh.material.transparent||!geo.attributes.normal)return;
  if(!geo.boundingBox)geo.computeBoundingBox();
  const dims=geo.boundingBox.getSize(new THREE.Vector3()).multiply(mesh.getWorldScale(new THREE.Vector3()));
  if(Math.min(dims.x,dims.y,dims.z)<.022)return;
  const hull=new THREE.Mesh(geo,outlineMaterial);hull.castShadow=hull.receiveShadow=false;hull.userData.outline=true;mesh.add(hull);
 });
 return g;
}

export function stageCreature(g,{disposition=null,normalize=true}={}){
 g.updateMatrixWorld(true);
 const box=new THREE.Box3(),inverse=g.matrixWorld.clone().invert(),local=new THREE.Box3();
 g.traverse(mesh=>{
  if(!mesh.isMesh||mesh.userData.outline)return;
  if(!mesh.geometry.boundingBox)mesh.geometry.computeBoundingBox();
  local.copy(mesh.geometry.boundingBox).applyMatrix4(mesh.matrixWorld).applyMatrix4(inverse);box.union(local);
 });
 if(box.isEmpty())return addOutlines(g);
 const gs=g.scale.x,world=box.getSize(new THREE.Vector3()).multiplyScalar(gs);
 if(normalize){
  const footprint=Math.max(world.x,world.z),current=Math.max(world.y,footprint);
  let s=Math.min(MAX_SCALE,Math.max(1,MIN_SIZE/current));
  s=Math.min(s,Math.max(1,MAX_FOOTPRINT/footprint));
  if(s>1.001){for(const child of g.children){child.scale.multiplyScalar(s);child.position.multiplyScalar(s);}box.min.multiplyScalar(s);box.max.multiplyScalar(s);world.multiplyScalar(s);g.updateMatrixWorld(true);}
 }
 addOutlines(g);
 if(disposition&&RINGS[disposition]){
  const ring=new THREE.Mesh(ringGeo,RINGS[disposition]);const d=Math.min(1,Math.max(.62,Math.max(world.x,world.z)*1.15));
  ring.scale.set(d/gs,1,d/gs);ring.position.set((box.min.x+box.max.x)/2,.02/gs,(box.min.z+box.max.z)/2);ring.renderOrder=-1;ring.userData.ring=true;g.add(ring);
 }
 return g;
}
