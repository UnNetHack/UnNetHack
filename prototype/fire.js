import * as THREE from 'three';

// Nested tapered tongues, animated independently rather than stretched spheres.
export function createFire(seed=0){
 const g=new THREE.Group(),tongues=[],embers=[];
 const materials=[0xff4b0c,0xff9e20,0xffebaa].map(color=>new THREE.MeshBasicMaterial({color,toneMapped:false}));
 const profile=[new THREE.Vector2(0,0),new THREE.Vector2(.055,.03),new THREE.Vector2(.072,.1),new THREE.Vector2(.05,.2),new THREE.Vector2(.023,.31),new THREE.Vector2(0,.43)];
 const geometry=new THREE.LatheGeometry(profile,10);
 for(let i=0;i<7;i++){
  const layer=i%3,mesh=new THREE.Mesh(geometry,materials[layer]);
  const size=layer===0?1:layer===1?.76:.48;
  mesh.position.set(i<3?0:Math.sin(i*2.4)*.065,.015+layer*.008,i<3?0:Math.cos(i*2.4)*.065);
  mesh.scale.setScalar(size);tongues.push({mesh,size,phase:seed+i*1.7});
 }
 const sparkGeometry=new THREE.SphereGeometry(.009,4,3);
 for(let i=0;i<8;i++){const mesh=new THREE.Mesh(sparkGeometry,materials[1]);g.add(mesh);embers.push(mesh);}
 const fireMaterial=new THREE.ShaderMaterial({transparent:true,depthWrite:false,side:THREE.DoubleSide,toneMapped:false,uniforms:{time:{value:0},seed:{value:seed}},
 vertexShader:`varying vec2 vUv;void main(){vUv=uv;vec4 c=modelViewMatrix*vec4(0.,0.,0.,1.);c.xy+=position.xy*vec2(length(modelMatrix[0].xyz),length(modelMatrix[1].xyz));gl_Position=projectionMatrix*c;}`,
 fragmentShader:`
 precision highp float;varying vec2 vUv;uniform float time,seed;
 float hash(vec2 p){return fract(sin(dot(p,vec2(127.1,311.7)))*43758.5453);}
 float noise(vec2 p){vec2 i=floor(p),f=fract(p);f=f*f*(3.-2.*f);return mix(mix(hash(i),hash(i+vec2(1,0)),f.x),mix(hash(i+vec2(0,1)),hash(i+vec2(1,1)),f.x),f.y);}
 float fbm(vec2 p){return .55*noise(p)+.28*noise(p*2.03)+.12*noise(p*4.09)+.05*noise(p*8.13);}
 void main(){
 float y=vUv.y;vec2 flow=vec2(vUv.x*5.+seed,y*4.-time*2.2);
 float curl=fbm(flow+vec2(0.,fbm(flow)*2.));
 float x=(vUv.x-.5)*2.+(curl-.5)*(.18+y*.58)+sin(y*9.-time*2.+seed)*y*.09;
 float density=.57*pow(max(0.,1.-y),.8)-abs(x)+(fbm(flow*1.65)-.5)*.27*y;
 float envelope=smoothstep(-.025,.09,density)*smoothstep(0.,.075,y)*(1.-smoothstep(.87,1.,y));
 float veins=fbm(vec2(x*15.+curl*3.,y*7.-time*4.));float filament=pow(1.-abs(veins*2.-1.),9.);
 float heat=clamp(density*1.9+(1.-y)*.2+filament*.42,0.,1.);
 vec3 color=mix(vec3(.65,.025,.002),vec3(1.6,.25,.008),smoothstep(0.,.36,heat));
 color=mix(color,vec3(2.1,1.1,.12),smoothstep(.32,.7,heat));color=mix(color,vec3(2.3,1.95,1.1),smoothstep(.72,1.,heat));
 float alpha=envelope*(.48+veins*.38+filament*.14);if(alpha<.008)discard;gl_FragColor=vec4(color,alpha);
 }`});
 const plane=new THREE.PlaneGeometry(.42,.64);plane.translate(0,.29,0);const flame=new THREE.Mesh(plane,fireMaterial);flame.frustumCulled=false;g.add(flame);
 g.userData.updateFire=t=>{
  fireMaterial.uniforms.time.value=t;
  for(const {mesh,size,phase} of tongues){const flicker=Math.sin(t*11+phase)*.12+Math.sin(t*19.3+phase)*.06;mesh.scale.set(size*(1-flicker*.4),size*(1+flicker),size);mesh.rotation.z=Math.sin(t*5.7+phase)*.2;mesh.rotation.x=Math.sin(t*7.1+phase)*.12;}
  embers.forEach((mesh,i)=>{const age=(t*(.45+i*.018)+i/8+seed)%1;mesh.position.set(Math.sin(age*8+i)*age*.17,.14+age*.65,Math.cos(age*7+i)*age*.12);mesh.scale.setScalar(Math.sin(age*Math.PI)*.8);});
 };
 g.userData.dispose=()=>{plane.dispose();fireMaterial.dispose();geometry.dispose();sparkGeometry.dispose();materials.forEach(m=>m.dispose());};
 return g;
}
