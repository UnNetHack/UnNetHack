import * as THREE from 'three';

// Procedural floor variety for Live mode. Every choice is keyed on the map square,
// so a tile looks the same each time the level is revisited.

export function cellHash(x,z,salt=0){let h=Math.imul(x,374761393)^Math.imul(z,668265263)^Math.imul(salt,2246822519);h=Math.imul(h^(h>>>13),1274126177);return (h^(h>>>16))>>>0;}

function rng(seed){let n=seed>>>0;return ()=>{n=(n*1664525+1013904223)>>>0;return n/4294967296;};}

const SIZE=256;

function paintBase(map,bump,r,{grain=1,worn=0}){
 map.fillStyle='#6b746f';map.fillRect(0,0,SIZE,SIZE);
 bump.fillStyle='#808080';bump.fillRect(0,0,SIZE,SIZE);
 for(let i=0;i<520;i++){
  const x=r()*SIZE,y=r()*SIZE,rad=2+r()*14,light=r()<.5;
  map.globalAlpha=(.03+r()*.09)*grain;map.fillStyle=light?'#8d948c':'#3f4845';
  map.beginPath();map.ellipse(x,y,rad,rad*(.4+r()*.6),r()*Math.PI,0,Math.PI*2);map.fill();
  bump.globalAlpha=(.04+r()*.08)*grain;bump.fillStyle=light?'#a0a0a0':'#606060';
  bump.beginPath();bump.ellipse(x,y,rad,rad*(.4+r()*.6),r()*Math.PI,0,Math.PI*2);bump.fill();
 }
 for(let i=0;i<1800*grain;i++){const x=r()*SIZE,y=r()*SIZE;map.globalAlpha=.08+r()*.14;map.fillStyle=r()<.5?'#a3a89f':'#2e3533';map.fillRect(x,y,1+r()*1.4,1+r()*1.4);}
 if(worn){
  const cx=SIZE*(.4+r()*.2),cy=SIZE*(.4+r()*.2);
  const g=map.createRadialGradient(cx,cy,0,cx,cy,SIZE*.55);g.addColorStop(0,`rgba(168,172,160,${.32*worn})`);g.addColorStop(1,'rgba(168,172,160,0)');
  map.globalAlpha=1;map.fillStyle=g;map.fillRect(0,0,SIZE,SIZE);
  const b=bump.createRadialGradient(cx,cy,0,cx,cy,SIZE*.55);b.addColorStop(0,`rgba(90,90,90,${.35*worn})`);b.addColorStop(1,'rgba(90,90,90,0)');
  bump.globalAlpha=1;bump.fillStyle=b;bump.fillRect(0,0,SIZE,SIZE);
 }
 // darker, slightly sunken rim where grime collects against the neighbours
 for(const ctx of [map,bump]){
  ctx.globalAlpha=1;
  for(const [x0,y0,x1,y1] of [[0,0,0,24],[0,SIZE,0,SIZE-24],[0,0,24,0],[SIZE,0,SIZE-24,0]]){
   const g=ctx.createLinearGradient(x0,y0,x1,y1);g.addColorStop(0,ctx===map?'rgba(30,36,34,.45)':'rgba(40,40,40,.5)');g.addColorStop(1,'rgba(0,0,0,0)');ctx.fillStyle=g;ctx.fillRect(0,0,SIZE,SIZE);
  }
 }
}

function crack(map,bump,r,{branches=2,width=1.8,length=26}){
 const edge=Math.floor(r()*4),t=20+r()*(SIZE-40);
 let [x,y]=edge===0?[t,0]:edge===1?[SIZE,t]:edge===2?[t,SIZE]:[0,t];
 let angle=Math.atan2(SIZE/2-y,SIZE/2-x)+(r()-.5)*.9;
 const walk=(x,y,angle,steps,w)=>{
  for(let i=0;i<steps;i++){
   const nx=x+Math.cos(angle)*(5+r()*9),ny=y+Math.sin(angle)*(5+r()*9);
   map.globalAlpha=.9;map.strokeStyle='#161b1a';map.lineWidth=w*2.2;map.beginPath();map.moveTo(x,y);map.lineTo(nx,ny);map.stroke();
   map.globalAlpha=.28;map.strokeStyle='#b2b7ad';map.lineWidth=w;map.beginPath();map.moveTo(x+2,y+2.4);map.lineTo(nx+2,ny+2.4);map.stroke();
   bump.globalAlpha=1;bump.strokeStyle='#141414';bump.lineWidth=w*3;bump.beginPath();bump.moveTo(x,y);bump.lineTo(nx,ny);bump.stroke();
   x=nx;y=ny;angle+=(r()-.5)*.8;w=Math.max(.6,w*.95);
   if(branches>0&&r()<.12){branches--;walk(x,y,angle+(r()<.5?-1:1)*(.6+r()*.6),Math.floor(steps*.5),w*.7);}
   if(x<0||y<0||x>SIZE||y>SIZE)break;
  }
 };
 walk(x,y,angle,length,width);
}

function chip(map,bump,r){
 const corner=Math.floor(r()*4),s=26+r()*22,pts=[[0,0],[s,0],[s*.55,s*.35],[s*.3,s*.7],[0,s]];
 const place=([px,py])=>corner===0?[px,py]:corner===1?[SIZE-px,py]:corner===2?[SIZE-px,SIZE-py]:[px,SIZE-py];
 for(const [ctx,fill] of [[map,'#39413e'],[bump,'#303030']]){ctx.globalAlpha=1;ctx.fillStyle=fill;ctx.beginPath();pts.map(place).forEach(([px,py],i)=>i?ctx.lineTo(px,py):ctx.moveTo(px,py));ctx.closePath();ctx.fill();}
}

function pits(map,bump,r,count){
 for(let i=0;i<count;i++){const x=12+r()*(SIZE-24),y=12+r()*(SIZE-24),rad=1.2+r()*3.2;
  map.globalAlpha=.3;map.fillStyle='#b5b9b0';map.beginPath();map.arc(x+.8,y+.8,rad,0,Math.PI*2);map.fill();
  map.globalAlpha=.8;map.fillStyle='#232927';map.beginPath();map.arc(x,y,rad,0,Math.PI*2);map.fill();
  bump.globalAlpha=1;bump.fillStyle='#202020';bump.beginPath();bump.arc(x,y,rad,0,Math.PI*2);bump.fill();}
}

function stain(map,r){
 const cx=40+r()*(SIZE-80),cy=40+r()*(SIZE-80);
 for(let i=0;i<7;i++){const x=cx+(r()-.5)*70,y=cy+(r()-.5)*70,rad=18+r()*38;const g=map.createRadialGradient(x,y,0,x,y,rad);g.addColorStop(0,'rgba(28,38,36,.32)');g.addColorStop(1,'rgba(28,38,36,0)');map.globalAlpha=1;map.fillStyle=g;map.fillRect(0,0,SIZE,SIZE);}
}

function moss(map,bump,r,amount){
 const corner=Math.floor(r()*4),[ox,oy]=[[0,0],[SIZE,0],[SIZE,SIZE],[0,SIZE]][corner];
 const colors=['#3f6a26','#52822f','#67973a','#35561f','#7aa14a'];
 for(let i=0;i<1500*amount;i++){
  const d=Math.pow(r(),1.8)*SIZE*.8*amount,a=r()*Math.PI*2,x=ox+Math.cos(a)*d,y=oy+Math.sin(a)*d;if(x<0||y<0||x>SIZE||y>SIZE)continue;
  const rad=1.5+r()*6*(1-d/(SIZE*.9));
  map.globalAlpha=.4+r()*.5;map.fillStyle=colors[Math.floor(r()*colors.length)];map.beginPath();map.arc(x,y,rad,0,Math.PI*2);map.fill();
  bump.globalAlpha=.3;bump.fillStyle='#b8b8b8';bump.beginPath();bump.arc(x,y,rad,0,Math.PI*2);bump.fill();
 }
}

// weight = how often the variant appears; plain stone dominates so damage stays special
const VARIANTS=[
 {weight:3,paint:(m,b,r)=>paintBase(m,b,r,{})},
 {weight:3,paint:(m,b,r)=>paintBase(m,b,r,{grain:1.3})},
 {weight:2,paint:(m,b,r)=>paintBase(m,b,r,{grain:.6,worn:1})},
 {weight:2,paint:(m,b,r)=>{paintBase(m,b,r,{});crack(m,b,r,{branches:1,width:1.4,length:20});}},
 {weight:1,paint:(m,b,r)=>{paintBase(m,b,r,{});crack(m,b,r,{branches:3,width:2.2,length:30});crack(m,b,r,{branches:1,width:1.4,length:16});chip(m,b,r);}},
 {weight:1,paint:(m,b,r)=>{paintBase(m,b,r,{});pits(m,b,r,34);}},
 {weight:1,paint:(m,b,r)=>{paintBase(m,b,r,{});stain(m,r);}},
 {weight:1,paint:(m,b,r)=>{paintBase(m,b,r,{});stain(m,r);moss(m,b,r,.7);}},
 {weight:.7,paint:(m,b,r)=>{paintBase(m,b,r,{grain:1.2});crack(m,b,r,{branches:2,width:1.8,length:26});moss(m,b,r,1);}},
];
const TOTAL_WEIGHT=VARIANTS.reduce((sum,v)=>sum+v.weight,0);
const TINTS=[[1,1,1],[.9,.92,.91],[1.07,1.05,1],[.83,.87,.88]];

export function createFloorKit(){
 const textures=[],materials=new Map();
 const texture=i=>{
  if(!textures[i]){
   const mc=document.createElement('canvas'),bc=document.createElement('canvas');mc.width=mc.height=bc.width=bc.height=SIZE;
   VARIANTS[i].paint(mc.getContext('2d'),bc.getContext('2d'),rng(0x9e3779b9+i*7919));
   const map=new THREE.CanvasTexture(mc),bump=new THREE.CanvasTexture(bc);map.colorSpace=THREE.SRGBColorSpace;map.anisotropy=bump.anisotropy=8;
   textures[i]={map,bump};
  }
  return textures[i];
 };
 const pebbleGeo=new THREE.DodecahedronGeometry(1,0),pebbleMat=new THREE.MeshStandardMaterial({color:'#4f5753',roughness:.95});
 function material(x,z){
  let pick=cellHash(x,z,1)/4294967296*TOTAL_WEIGHT,variant=0;
  while(pick>=VARIANTS[variant].weight){pick-=VARIANTS[variant].weight;variant++;}
  const tint=cellHash(x,z,2)%TINTS.length,key=variant*16+tint;
  if(!materials.has(key)){const {map,bump}=texture(variant);materials.set(key,new THREE.MeshStandardMaterial({color:new THREE.Color(...TINTS[tint]),map,bumpMap:bump,bumpScale:1.6,roughness:.9}));}
  return materials.get(key);
 }
 // Orient, settle and scatter debris on the floor slab of one map square.
 function dress(slab,tile,x,z,terrain){
  const h=cellHash(x,z,3);
  slab.rotation.y=(h&3)*Math.PI/2;
  slab.position.y+=((h>>>2&15)/15-.5)*.014;
  if(terrain!=='floor'||cellHash(x,z,4)%11)return;
  const r=rng(cellHash(x,z,5)),count=1+Math.floor(r()*4);
  for(let i=0;i<count;i++){const p=new THREE.Mesh(pebbleGeo,pebbleMat),s=.022+r()*.03;p.scale.set(s*(1+r()*.5),s*.55,s);p.position.set((r()-.5)*.7,-.03+s*.4,(r()-.5)*.7);p.rotation.set(r()*3,r()*3,r()*3);p.receiveShadow=true;tile.add(p);}
 }
 return {material,dress};
}
