import * as THREE from 'three';
import {createHeldWeapon} from './equipment.js';
import {createAltar} from './altar.js';
import {createFire} from './fire.js';
import {createFloorKit,cellHash} from './floor.js';
import {stageCreature,addOutlines} from './readability.js';
import {createCavern} from './cavern.js';
import { RoundedBoxGeometry } from 'three/addons/geometries/RoundedBoxGeometry.js';

// Only window-port observations enter this view. No prediction of game rules.
export function installLive({scene,camera,controls,playerFactory,catFactory,monsterFactory,creatureFactory,wellTemplate,demoObjects,onDemo,onMode}) {
 const group=new THREE.Group();scene.add(group);group.visible=false;
 const tiles=new Map(),actors=new Map(),wells=new Map(),groundItems=new Map();let active=false,pending=null,latest=null,token='',menu=null,lines=[],origin=null,lastLevel='',source;
 const $=s=>document.querySelector(s);
 const WEAPON_CLASS=2,ARMOR_CLASS=3,RING_CLASS=4,AMULET_CLASS=5,POTION_CLASS=8,SCROLL_CLASS=9,COIN_CLASS=12;
 const button=document.createElement('button');button.textContent='Live UnNetHack';button.id='live-mode';$('.buttons').prepend(button);
 const panel=document.createElement('section');panel.id='engine-panel';panel.hidden=true;panel.innerHTML='<small>UNNETHACK · LIVE ENGINE</small><p id="engine-line" aria-live="polite"></p><div id="engine-messages" role="log"></div><div id="engine-status"></div><div id="engine-seen"></div><div id="engine-prompt"></div>';document.body.append(panel);
 // Engine commands live in the footer next to the demo's buttons, so both modes share one control row.
 const actions=document.createElement('div');actions.className='engine-actions';actions.hidden=true;actions.innerHTML='<button data-key="105">Inventory</button><button data-key="44">Pick up</button><button data-key="111">Open door</button><button data-key="113">Quaff</button><button data-key="83">Save & exit</button>';$('.buttons').prepend(actions);
 const esc=text=>String(text).replace(/[&<>"]/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;'}[c]));
 function setPrompt(text){$('#engine-prompt').innerHTML=text?`<span class="dot"></span>${esc(text)}`:'';}
 // Parse the bottom status line into the same label/value pairs the character panel uses.
 function renderStatus(text){
   const el=$('#engine-status'),num=re=>text.match(re);el.title=text;
   const gold=num(/\$:(\d+)/),power=num(/Pw:(\d+)\((\d+)\)/),exp=num(/Exp:(\d+)/);
   if(!gold&&!power&&!exp){el.textContent=text;return;}
   const conditions=(text.split(/T:\d+/)[1]||'').trim().split(/\s+/).filter(Boolean);
   el.innerHTML=[gold&&`<span>GOLD <b>${gold[1]}</b></span>`,power&&`<span>POWER <b>${power[1]} / ${power[2]}</b></span>`,exp&&`<span>EXP <b>${exp[1]}</b></span>`,...conditions.map(c=>`<span class="condition">${esc(c)}</span>`)].filter(Boolean).join('');
 }
 // Mirror the demo legend: what is in view, as dot bullets; pets go to the companion slot.
 function renderSurroundings(frame){
   const seenItems=[],pets=[],names=new Set();
   for(const cell of frame.cells){
     if(!cell.visible||(cell.x===frame.player.x&&cell.z===frame.player.z&&cell.kind!=='object'))continue;
     if(cell.kind==='pet')pets.push(cell.name);
     else if(cell.kind==='monster'&&!names.has(cell.name)){names.add(cell.name);seenItems.push({name:cell.name,tone:cell.peaceful?'gold':'orange'});}
     else if(['fountain','altar','up','down'].includes(cell.terrain)&&!names.has(cell.terrain)){names.add(cell.terrain);seenItems.push({name:{fountain:'Fountain',altar:'Altar',up:'Stairs up',down:'Stairs down'}[cell.terrain],tone:'cyan'});}
   }
   $('#engine-seen').innerHTML=seenItems.length?seenItems.slice(0,5).map(({name,tone})=>`<div><i class="${tone}"></i> ${esc(name)}</div>`).join(''):'<div class="quiet">Nothing stirs in view</div>';
   $('.companion').innerHTML=pets.length?`<span class="dot"></span> ${esc(pets[0])}${pets.length>1?` +${pets.length-1}`:''}<small>YOUR COMPANION · UNNETHACK</small>`:'<span class="dot faded"></span> Alone<small>NO COMPANION IN SIGHT</small>';
 }
 const dialog=document.createElement('dialog');dialog.id='engine-dialog';document.body.append(dialog);
 const lantern=new THREE.PointLight(0xffd49c,15,9,2);group.add(lantern);
 const stone=new THREE.MeshStandardMaterial({color:'#68736e',roughness:.9}),wall=new THREE.MeshStandardMaterial({color:'#52605f',roughness:.88}),wood=new THREE.MeshStandardMaterial({color:'#95774e',roughness:.8}),doorFace=new THREE.MeshStandardMaterial({color:'#4d382a',roughness:.9}),iron=new THREE.MeshStandardMaterial({color:'#293337',metalness:.72,roughness:.4}),floorGeo=new RoundedBoxGeometry(.97,.14,.97,3,.035),wallGeo=new RoundedBoxGeometry(.97,.7,.97,3,.045),doorGeo=new RoundedBoxGeometry(.86,1.1,.16,3,.025),stepGeo=new RoundedBoxGeometry(.76,.14,.22,3,.025);
 // Torches are real light sources: a fixed pool of point lights follows the torches
 // nearest the hero. The pool size never changes, so materials never recompile.
 const TORCH_LIGHTS=8,TORCH_INTENSITY=15,LIVE_AMBIENT=.4;
 const torchLights=Array.from({length:TORCH_LIGHTS},()=>{const l=new THREE.PointLight(0xff9a48,0,6,2);l.userData={tile:null,level:0};group.add(l);return l;});
 const ambientLights=scene.children.filter(o=>o.isHemisphereLight||o.isDirectionalLight).map(light=>({light,base:light.intensity}));
 const torchHaloMaterial=(()=>{const c=document.createElement('canvas');c.width=c.height=64;const ctx=c.getContext('2d'),g=ctx.createRadialGradient(32,32,0,32,32,32);g.addColorStop(0,'rgba(255,190,110,.55)');g.addColorStop(.35,'rgba(255,130,50,.18)');g.addColorStop(1,'rgba(255,100,30,0)');ctx.fillStyle=g;ctx.fillRect(0,0,64,64);return new THREE.SpriteMaterial({map:new THREE.CanvasTexture(c),blending:THREE.AdditiveBlending,depthWrite:false,toneMapped:false});})();
 // Integer hash so torches scatter along a wall instead of lining a whole row.
 function hasTorch(x,z){return cellHash(x,z)%9===0;}
 const floorKit=createFloorKit();
 const cavern=createCavern({group,scene,camera,controls});
 function updateTorchLights(t,dt){
   const rank=tile=>tile.position.distanceToSquared(hero.g.position)+(tile.userData.fog.visible?16:0);
   const wanted=new Set([...tiles.values()].filter(tile=>tile.userData.torch).sort((a,b)=>rank(a)-rank(b)).slice(0,TORCH_LIGHTS));
   const held=new Set(torchLights.map(l=>l.userData.tile));
   for(const tile of wanted){if(held.has(tile))continue;const free=torchLights.find(l=>!l.userData.tile);if(!free)break;free.userData.tile=tile;free.userData.level=0;free.position.copy(tile.position).setY(1.15);}
   const ease=1-Math.exp(-dt*5);
   for(const l of torchLights){const {tile}=l.userData;if(!tile){l.intensity=0;continue;}
     if(!tile.parent){l.userData.tile=null;l.intensity=0;continue;}
     const target=wanted.has(tile)?(tile.userData.fog.visible?.55:1):0;l.userData.level+=(target-l.userData.level)*ease;
     if(!target&&l.userData.level<.02){l.userData.tile=null;l.intensity=0;continue;}
     const p=tile.userData.torch.phase,flicker=1+Math.sin(t*11+p)*.09+Math.sin(t*19.3+p*1.7)*.05+Math.sin(t*3.1+p)*.04;
     l.intensity=TORCH_INTENSITY*l.userData.level*flicker;
   }
 }
 function box(geo,mat,parent,x,y,z){const m=new THREE.Mesh(geo,mat);m.position.set(x,y,z);m.castShadow=m.receiveShadow=true;parent.add(m);return m;}
 function label(text,color='#f0d9b0'){const c=document.createElement('canvas');c.width=512;c.height=96;const ctx=c.getContext('2d');ctx.fillStyle='rgba(10,20,20,.68)';ctx.beginPath();ctx.roundRect(54,16,404,64,12);ctx.fill();ctx.font='30px system-ui';ctx.textAlign='center';ctx.fillStyle=color;ctx.fillText(text,256,61);const texture=new THREE.CanvasTexture(c),material=new THREE.SpriteMaterial({map:texture,depthTest:false});const s=new THREE.Sprite(material);s.scale.set(1.3,.25,1);s.position.y=1.7;s.userData.dispose=()=>{texture.dispose();material.dispose();};return s;}
 function release(object){object.traverse(o=>o.userData.dispose?.());group.remove(object);}
 function clear(){for(const o of tiles.values())release(o);for(const a of actors.values())release(a.g);for(const o of groundItems.values())release(o);for(const w of wells.values())group.remove(w);tiles.clear();actors.clear();groundItems.clear();wells.clear();}
 function pickupIcon(cell){
   const icon=new THREE.Group(), kind=cell.object?.kind||'item', cls=cell.object?.class||0, itemName=(cell.object?.name||cell.name||'').toLowerCase();
   const warm=new THREE.MeshStandardMaterial({color:kind==='corpse'?0x72534a:cls===POTION_CLASS?0x5bd0c7:cls===WEAPON_CLASS?0xd9b15e:0xc9a86b,emissive:kind==='corpse'?0x241314:0x362718,roughness:.42,metalness:cls===WEAPON_CLASS?.65:.18});
   const edge=new THREE.MeshStandardMaterial({color:kind==='corpse'?0xb9a189:0xe8d8aa,roughness:.55,metalness:cls===WEAPON_CLASS?.7:.25});
   const add=(geometry,material=warm,x=0,y=.34,z=0)=>{const m=new THREE.Mesh(geometry,material);m.position.set(x,y,z);m.castShadow=true;icon.add(m);return m;};
   if(kind==='corpse'){
     const bone=new THREE.MeshStandardMaterial({color:0xc9b993,roughness:.78}),boneShade=new THREE.MeshStandardMaterial({color:0x756a5c,roughness:.86}),socket=new THREE.MeshStandardMaterial({color:0x17191a,roughness:1});
     const addBone=(angle,x,z,length=.48)=>{const shaft=add(new THREE.CylinderGeometry(.035,.035,length,8),bone,x,.31,z);shaft.rotation.z=Math.PI/2;shaft.rotation.y=angle;for(const end of [-1,1]){const p=add(new THREE.SphereGeometry(.07,8,6),bone,x+Math.cos(angle)*length*.5*end,.31,z+Math.sin(angle)*length*.5*end);p.scale.set(1,.72,1);}};
     addBone(.18,-.08,-.02,.55);addBone(-.92,.08,.02,.52);addBone(1.25,0,-.08,.4);addBone(-.28,.02,.1,.34);
     const skull=add(new THREE.SphereGeometry(.16,12,8),bone,0,.49,.02);skull.scale.set(.92,.82,1.08);add(new THREE.BoxGeometry(.14,.06,.13),boneShade,0,.39,.12);
     for(const x of [-.055,.055])add(new THREE.SphereGeometry(.026,8,6),socket,x,.51,.155);
     icon.userData.dispose=()=>{bone.dispose();boneShade.dispose();socket.dispose();};
   } else if(/boulder|large rock/.test(itemName)){
     const rock=new THREE.MeshStandardMaterial({color:0x5b5752,roughness:.97}),rockLight=new THREE.MeshStandardMaterial({color:0x81786c,roughness:.92}),rockDark=new THREE.MeshStandardMaterial({color:0x292b2a,roughness:1}),rockShadow=new THREE.MeshBasicMaterial({color:0x111517,transparent:true,opacity:.58,depthWrite:false});
     const ground=add(new THREE.CircleGeometry(.34,24),rockShadow,0,.002,0);ground.rotation.x=-Math.PI/2;
     const mass=add(new THREE.DodecahedronGeometry(.3,2),rock,0,.22,0);mass.scale.set(.98,.76,1.12);mass.rotation.set(.08,-.32,.12);
     const shoulder=add(new THREE.DodecahedronGeometry(.16,1),rockLight,-.18,.13,.11);shoulder.scale.set(1,.62,.78);shoulder.rotation.set(.2,.5,-.15);const chip=add(new THREE.DodecahedronGeometry(.11,1),rockDark,.2,.1,-.12);chip.scale.set(1,.55,.8);chip.rotation.set(.3,-.2,.4);
     icon.userData.dispose=()=>{rock.dispose();rockLight.dispose();rockDark.dispose();rockShadow.dispose();};
   } else if(cls===WEAPON_CLASS){
     const weapon=createHeldWeapon({name:itemName});
     const pose=new THREE.Group();pose.rotation.y=-.65;icon.add(pose);
     weapon.rotation.x=-Math.PI/2;weapon.scale.setScalar(.8);weapon.position.set(0,.052,.3);pose.add(weapon);
     icon.userData.restingWeapon=true;
   } else if(cls===COIN_CLASS){
     const coinMats=[new THREE.MeshStandardMaterial({color:0xb78c38,metalness:.72,roughness:.42,emissive:0x2b1b06,emissiveIntensity:.07}),new THREE.MeshStandardMaterial({color:0xc59b43,metalness:.76,roughness:.38,emissive:0x301e07,emissiveIntensity:.08}),new THREE.MeshStandardMaterial({color:0xa98032,metalness:.7,roughness:.46,emissive:0x241705,emissiveIntensity:.06})],coinStampMats=[new THREE.MeshStandardMaterial({color:0xd2b45f,metalness:.66,roughness:.48}),new THREE.MeshStandardMaterial({color:0xe0c675,metalness:.7,roughness:.44})],shadow=new THREE.MeshBasicMaterial({color:0x140e08,transparent:true,opacity:.48,depthWrite:false});
     const ground=add(new THREE.CircleGeometry(.31,24),shadow,0,.002,0);ground.rotation.x=-Math.PI/2;
     const pieces=[[-.18,-.11,0,.2,0,.05],[-.07,-.15,0,-.4,0,0],[.06,-.14,0,.7,.04,-.03],[.17,-.06,0,-.1,0,0],[-.2,.01,0,.5,0,.08],[-.1,.01,1,-.6,.08,0],[.02,-.02,1,.4,0,0],[.15,.03,0,-.3,0,-.06],[-.16,.1,0,.8,0,0],[-.04,.11,1,.1,.05,.04],[.08,.1,1,-.5,0,0],[.2,.1,0,.3,0,.08],[-.04,-.04,2,.6,0,0],[-.13,-.03,1,-.2,.1,-.04],[.07,.01,2,.5,0,0],[.02,.12,2,-.8,.06,0],[.12,-.08,1,.15,0,-.08],[-.22,-.06,0,-.5,0,.04]];
     const coinPile=[];
     for(const [i,[x,z,layer,rotation,tiltX,tiltZ]] of pieces.entries()){const y=.025+layer*.027,fallStart=y+.42+layer*.06,coin=coinMats[i%coinMats.length],coinStamp=coinStampMats[i%coinStampMats.length];const disk=add(new THREE.CylinderGeometry(.078,.078,.032,16),coin,x,fallStart,z);disk.rotation.set(tiltX,rotation,tiltZ);const stamp=add(new THREE.CylinderGeometry(.026,.026,.004,10),coinStamp,x,fallStart+.019,z);stamp.rotation.set(tiltX,rotation,tiltZ);coinPile.push({disk,stamp,target:y,velocity:0,delay:layer*.09+(x+.3)*.018,settled:false});}
     icon.userData.coinPile=coinPile;
     icon.userData.dispose=()=>{coinMats.forEach(material=>material.dispose());coinStampMats.forEach(material=>material.dispose());shadow.dispose();};
   } else if(cls===POTION_CLASS){
     const glass=new THREE.MeshPhysicalMaterial({color:0x9dd9d1,roughness:.16,metalness:.02,transmission:.2,transparent:true,opacity:.58}),liquid=new THREE.MeshStandardMaterial({color:0x43b9b7,emissive:0x0b5555,emissiveIntensity:.35,roughness:.3});
     const bottle=add(new THREE.SphereGeometry(.18,16,10),glass,0,.3,0);bottle.scale.y=1.18;const fill=add(new THREE.SphereGeometry(.145,14,9),liquid,0,.28,0);fill.scale.y=1.18;add(new THREE.CylinderGeometry(.065,.065,.14,10),glass,0,.57,0);add(new THREE.CylinderGeometry(.075,.085,.045,12),edge,0,.66,0);
     icon.userData.dispose=()=>{glass.dispose();liquid.dispose();};
   } else if(cls===ARMOR_CLASS){
     const helm=new THREE.MeshStandardMaterial({color:0x89979a,metalness:.78,roughness:.34}),visorMat=new THREE.MeshStandardMaterial({color:0x182126,metalness:.5,roughness:.3});const dome=add(new THREE.SphereGeometry(.27,16,10),helm,0,.32,0);dome.scale.set(.86,.7,.7);add(new THREE.BoxGeometry(.08,.42,.06),visorMat,0,.36,.19);add(new THREE.ConeGeometry(.06,.2,5),edge,0,.57,0);
     icon.userData.dispose=()=>{helm.dispose();visorMat.dispose();};
   } else if(cls===RING_CLASS){
     const jewelry=new THREE.MeshStandardMaterial({color:0xb49a5e,metalness:.8,roughness:.3}),ring=add(new THREE.TorusGeometry(.16,.045,8,20),jewelry,0,.3,0);ring.rotation.x=Math.PI/2;const highlight=add(new THREE.SphereGeometry(.045,8,6),jewelry,0,.3,.15);highlight.scale.set(1,.7,.6);
     icon.userData.dispose=()=>jewelry.dispose();
   } else if(cls===AMULET_CLASS){
     const jewelry=new THREE.MeshStandardMaterial({color:0xb49a5e,metalness:.78,roughness:.32}),chain=add(new THREE.TorusGeometry(.15,.018,6,18),jewelry,0,.47,0);chain.rotation.x=Math.PI/2;const pendant=add(new THREE.OctahedronGeometry(.1),jewelry,0,.27,.02);pendant.scale.y=1.25;
     icon.userData.dispose=()=>jewelry.dispose();
   } else if(cls===SCROLL_CLASS){
     const parchment=new THREE.MeshStandardMaterial({color:0xcbb889,roughness:.72}),ribbon=new THREE.MeshStandardMaterial({color:0x714243,roughness:.62});const roll=add(new THREE.CylinderGeometry(.13,.13,.42,12),parchment,0,.36,0);roll.rotation.z=Math.PI/2;for(const x of [-.21,.21]){const end=add(new THREE.CylinderGeometry(.15,.15,.035,12),parchment,x,.36,0);end.rotation.z=Math.PI/2;}add(new THREE.BoxGeometry(.045,.08,.11),ribbon,0,.36,.0);
     icon.userData.dispose=()=>{parchment.dispose();ribbon.dispose();};
   } else {
     add(new THREE.OctahedronGeometry(.2),warm,0,.38,0);
   }
   const caption=label(kind==='corpse'?`corpse of ${cell.name||'creature'}`:(cell.name||'item'),'#d7c8a7');caption.scale.set(1.2,.22,1);caption.position.y=1.05;icon.add(caption);const extraDispose=icon.userData.dispose;icon.userData.dispose=()=>{warm.dispose();edge.dispose();extraDispose?.();};return icon;
 }
 function setDim(tile,dim){tile.userData.fog.visible=dim;tile.userData.fog.material.opacity=dim?.72:0;tile.userData.fog.material.needsUpdate=true;}
 const hero=playerFactory();hero.setWeapon?.(null);group.add(hero.g);
 function apply(frame){latest=frame;if(!active)return;
   hero.setWeapon?.(frame.player.weapon??null);
   hero.setHelmet?.(frame.player.helmet??null);addOutlines(hero.g);
   const level=`${frame.branch}:${frame.depth}`;const newLevel=level!==lastLevel;if(newLevel){clear();origin={x:frame.player.x,z:frame.player.z};lastLevel=level;hero.g.position.set(0,0,0);camera.position.set(9,10.7,13.1);controls.target.set(0,0,0);}
   const seen=new Set(),seenActors=new Set(),seenWells=new Set();
   for(const cell of frame.cells){const id=`${cell.x},${cell.z}`,x=cell.x-origin.x,z=cell.z-origin.z;
     if(cell.terrain!=='unknown'){
       seen.add(id);let tile=tiles.get(id);if(tile&&tile.userData.type!==cell.terrain){release(tile);tiles.delete(id);tile=null;}
       if(!tile){tile=new THREE.Group();tile.position.set(x,0,z);tile.userData.type=cell.terrain;floorKit.dress(box(floorGeo,floorKit.material(cell.x,cell.z),tile,0,-.1,0),tile,cell.x,cell.z,cell.terrain);const fog=box(new THREE.PlaneGeometry(.98,.98),new THREE.MeshBasicMaterial({color:0x101a35,transparent:true,opacity:0,depthWrite:false}),tile,0,.012,0);fog.rotation.x=-Math.PI/2;tile.userData.fog=fog;
         if(cell.terrain==='feature'){const s=label(String.fromCharCode(cell.symbol));s.position.y=.35;tile.add(s);}
         if(cell.terrain==='altar')tile.add(createAltar());
         if(cell.terrain==='bars'){
          const grate=new THREE.Group();grate.name='Iron bars';tile.add(grate);tile.userData.grate=grate;
          const owned=[];
          const bar=(w,h,d,x,y,z)=>{const geo=new RoundedBoxGeometry(w,h,d,2,.009);owned.push(geo);return box(geo,iron,grate,x,y,z);};
          for(const x of [-.44,.44])bar(.085,.95,.14,x,.475,0);
          for(const y of [.08,.48,.88])bar(.94,.055,.09,0,y,0);
          for(let i=0;i<7;i++){
           const x=(i-3)*.12;bar(.038,.84,.04,x,.46,0);
           for(const y of [.08,.48,.88])bar(.058,.075,.065,x,y,0);
          }
          grate.userData.dispose=()=>owned.forEach(geo=>geo.dispose());
         }
         if(cell.terrain==='wall'){
          box(wallGeo,wall,tile,0,.28,0);
          if(hasTorch(cell.x,cell.z)){box(new THREE.CylinderGeometry(.045,.055,.3,8),wood,tile,0,.77,0);box(new THREE.CylinderGeometry(.09,.055,.1,8),iron,tile,0,.92,0);const fire=createFire(cell.x+cell.z);fire.position.y=.96;tile.add(fire);const halo=new THREE.Sprite(torchHaloMaterial);halo.position.y=1.12;halo.scale.setScalar(.9);tile.add(halo);tile.userData.torch={phase:(cell.x*3.7+cell.z*5.3)%(Math.PI*2)};}
         }
         if(cell.terrain==='door'){box(doorGeo,doorFace,tile,0,.52,0);for(const x of [-.23,.23]){const brace=box(new RoundedBoxGeometry(.055,.92,.035,3,.012),iron,tile,x,.52,.1);brace.rotation.z=x<0?-.38:.38;}const latch=new THREE.Mesh(new THREE.SphereGeometry(.06,10,8),iron);latch.position.set(.18,.55,.14);latch.castShadow=true;tile.add(latch);}
         if(cell.terrain==='up'||cell.terrain==='down'){const direction=cell.terrain==='up'?1:-1;for(let i=0;i<5;i++){const z=(i-2)*.16*direction,y=.045+i*.085;box(stepGeo,i%2?stone:wall,tile,0,y,z);for(const x of [-.3,.3]){const base=new THREE.Mesh(new THREE.CylinderGeometry(.07,.08,.035,8),iron);base.position.set(x,y+.1,z);base.castShadow=true;tile.add(base);const spike=new THREE.Mesh(new THREE.ConeGeometry(.052,.25,6),iron);spike.position.set(x,y+.24,z);spike.castShadow=true;tile.add(spike);}}tile.add(label(cell.terrain==='up'?'↑ stone stairs':'↓ stone stairs'));}
         if(['water','lava'].includes(cell.terrain)){const m=new THREE.MeshStandardMaterial({color:cell.terrain==='water'?0x247c89:0xd85820,emissive:cell.terrain==='water'?0x103640:0x852400,roughness:.25});const s=box(floorGeo,m,tile,0,.01,0);s.userData.dispose=()=>m.dispose();tile.userData.liquid=s;tile.userData.liquidPhase=(x*7+z*13)%6;}
         group.add(tile);tiles.set(id,tile);
       }tile.visible=true;tile.scale.y=1;setDim(tile,!cell.visible&&cell.remembered);
       if(tile.userData.grate){
        const connected=(dx,dz)=>frame.cells.some(c=>c.x===cell.x+dx&&c.z===cell.z+dz&&['wall','bars','door'].includes(c.terrain));
        const horizontal=Number(connected(-1,0))+Number(connected(1,0)),vertical=Number(connected(0,-1))+Number(connected(0,1));
        if(horizontal!==vertical)tile.userData.grate.rotation.y=vertical>horizontal?Math.PI/2:0;
       }
     }
     if(cell.terrain==='fountain'){seenWells.add(id);if(!wells.has(id)){const w=wellTemplate.clone(true);w.position.set(x,0,z);group.add(w);wells.set(id,w);}}
     if(cell.x===frame.player.x&&cell.z===frame.player.z)continue;
       if(cell.kind==='object'){
       const key=`${id}:${cell.glyph}`,seenObject=cell.visible||cell.remembered;seenActors.add(key);
       if(seenObject&&!groundItems.has(key)){const item=pickupIcon(cell);item.position.set(x,0,z);group.add(item);groundItems.set(key,item);}
       const item=groundItems.get(key);if(item){item.visible=cell.visible;if(!item.userData.coinPile&&!item.userData.restingWeapon)item.position.y=Math.sin(performance.now()/600+x+z)*.025;}
     }
       if(cell.kind==='monster'||cell.kind==='pet'){
       const key=`${id}:${cell.glyph}`;seenActors.add(key);let a=actors.get(key);
       if(!a){for(const [previous,candidate] of actors){if(!seenActors.has(previous)&&candidate.glyph===cell.glyph&&Math.hypot(candidate.g.position.x-x,candidate.g.position.z-z)<2.1){a=candidate;actors.delete(previous);actors.set(key,a);break;}}}
       if(!a){const disposition=cell.kind==='pet'?'pet':cell.peaceful?'peaceful':'hostile';if(cell.kind==='pet'&&/cat|kitten/.test(cell.name)){a=catFactory();stageCreature(a.g,{disposition});a.g.add(label(cell.name,'#b8ead3'));}else{const made=creatureFactory?creatureFactory(cell):monsterFactory();a=made.g?made:{g:made};stageCreature(a.g,{disposition});a.g.add(label(cell.name||'creature',cell.kind==='pet'?'#b8ead3':cell.peaceful?'#e8dfb0':'#e9c8ad'));}a.g.position.set(x,0,z);group.add(a.g);actors.set(key,a);}
       a.glyph=cell.glyph;a.species=(cell.name||'').toLowerCase();a.target=new THREE.Vector3(x,0,z);
     }
   }
   for(const [id,t] of tiles)if(!seen.has(id)){release(t);tiles.delete(id);}
   cavern.rebuild(tiles,origin,newLevel);
   for(const [id,a] of actors)if(!seenActors.has(id)){release(a.g);actors.delete(id);}
   for(const [id,item] of groundItems)if(!seenActors.has(id)){release(item);groundItems.delete(id);}
   for(const [id,w] of wells)if(!seenWells.has(id)){group.remove(w);wells.delete(id);}
   hero.target=new THREE.Vector3(frame.player.x-origin.x,0,frame.player.z-origin.z);renderSurroundings(frame);
   $('#hp').textContent=`${frame.player.hp} / ${frame.player.maxhp}`;$('#healthbar').style.width=`${100*frame.player.hp/Math.max(1,frame.player.maxhp)}%`;$('#turn').textContent=frame.turn;$('.stats').innerHTML=`<span>AC <b>${frame.player.ac}</b></span><span>LVL <b>${frame.player.level}</b></span><span>TURN <b id="turn">${frame.turn}</b></span>`;$('.location h1').textContent=`The Dungeons · ${frame.depth}`;
 }
 function message(text){const line=$('#engine-line'),log=$('#engine-messages');if(line.textContent){const p=document.createElement('div');p.textContent=line.textContent;log.prepend(p);while(log.children.length>3)log.lastChild.remove();}line.textContent=text;$('#message').textContent=text;}
 async function post(path,body={}){if(!token)token=(await fetch('/engine/token').then(r=>r.json())).token;const r=await fetch(path,{method:'POST',headers:{'Content-Type':'application/json','X-Engine-Token':token},body:JSON.stringify(body)});const data=await r.json();if(!r.ok)throw new Error(data.error);return data;}
 async function reply(value){if(!pending)return;const req=pending;pending=null;lines=[];menu=null;dialog.close();setPrompt('Engine is resolving your action…');try{await post('/engine/input',{id:req.id,value});}catch(e){message(e.message);setPrompt('Input was not accepted. Reconnect Live mode to refresh the prompt.');}}
 function prompt(){if(!active||!pending)return;setPrompt(pending.prompt);if(pending.kind==='command')return;
   dialog.replaceChildren();const kicker=document.createElement('small');kicker.textContent='UNNETHACK ASKS';const title=document.createElement('h2');title.textContent=pending.prompt||'UnNetHack';dialog.append(kicker,title);
   if(pending.kind==='menu'&&menu){const form=document.createElement('form');for(const item of menu.items){const row=document.createElement('label');row.className='engine-menu-row';if(item.selectable&&menu.how!==0){const input=document.createElement('input');input.type=menu.how===1?'radio':'checkbox';input.name='selection';input.value=item.id;input.dataset.accelerator=item.accelerator||'';row.append(input);const accel=document.createElement('kbd');accel.textContent=item.accelerator?`[${item.accelerator}]`:'';row.append(accel);}row.append(document.createTextNode(item.text));form.append(row);}const submit=document.createElement('button');submit.textContent='Continue (Enter)';submit.type='submit';form.append(submit);form.onsubmit=e=>{e.preventDefault();reply([...form.querySelectorAll('input:checked')].map(i=>i.value).join(','));};form.addEventListener('keydown',e=>{const key=e.key.length===1?e.key.toLowerCase():e.key;if(key==='Escape'){e.preventDefault();reply('!');return;}if(key==='Enter'){e.preventDefault();form.requestSubmit();return;}if(key==='a'&&menu.how===2){e.preventDefault();form.querySelectorAll('input').forEach(i=>i.checked=true);return;}const input=[...form.querySelectorAll('input')].find(i=>i.dataset.accelerator===key);if(input){e.preventDefault();if(menu.how===1)reply(input.value);else input.checked=!input.checked;}});dialog.append(form);}
   else if(pending.kind==='line'){const form=document.createElement('form'),input=document.createElement('input'),submit=document.createElement('button');input.maxLength=200;input.autofocus=true;submit.textContent='Enter';form.append(input,submit);form.onsubmit=e=>{e.preventDefault();reply(input.value);};dialog.append(form);}
   else{if(lines.length){const pre=document.createElement('pre');pre.textContent=lines.join('\n');dialog.append(pre);}const p=document.createElement('p');p.textContent=pending.kind==='more'?'Press Enter to continue.':'Press a response key. For a direction use arrows or h/j/k/l.';dialog.append(p);const ok=document.createElement('button');ok.textContent='Enter';ok.onclick=()=>reply(13);dialog.append(ok);}
   const cancel=document.createElement('button');cancel.textContent='Cancel / Escape';cancel.onclick=()=>reply(pending.kind==='menu'?'!':pending.kind==='line'?'\u001b':27);dialog.append(cancel);dialog.showModal();
 }
 dialog.addEventListener('cancel',e=>{e.preventDefault();if(pending)reply(pending.kind==='menu'?'!':pending.kind==='line'?'\u001b':27);});
 function connect(){source?.close();source=new EventSource('/engine/events');source.onmessage=e=>{const v=JSON.parse(e.data);if(v.type==='frame')apply(v);else if(v.type==='request'){pending=v;prompt();}else if(v.type==='message'){if(active)message(v.text);}else if(v.type==='status')renderStatus(v.text);else if(v.type==='menu')menu=v;else if(v.type==='text')lines=v.lines;else if(v.type==='ended'){pending=null;if(active){dialog.close();message(v.text);setPrompt('Session ended. Use Demo room, then Live UnNetHack to resume.');}}};source.onerror=()=>{if(active)setPrompt('Connection interrupted; reconnecting…');};}
 const saved={heading:$('.location h1').textContent,footer:$('footer>small').textContent,keys:$('.keys').innerHTML,companion:$('.companion').innerHTML};
 function setMode(value){active=value;cavern.setActive(active);for(const {light,base} of ambientLights)light.intensity=active?base*LIVE_AMBIENT:base;document.body.classList.toggle('live-engine',active);group.visible=active;for(const o of demoObjects)o.visible=!active;panel.hidden=!active;actions.hidden=!active;$('#reset').hidden=active;$('.legend').hidden=active;$('.character h2').hidden=active;button.textContent=active?'Demo room':'Live UnNetHack';if(!active)$('.companion').innerHTML=saved.companion;$('footer>small').textContent=active?'Real UnNetHack rules · isolated character and saves · drag to orbit, scroll to zoom':saved.footer;$('.keys').innerHTML=active?'<span><kbd>h j k l / arrows</kbd> Move</span><span><kbd>y u b n</kbd> Diagonals</span><span><kbd>s</kbd> Search</span><span><kbd>SPACE</kbd> Wait</span><span><kbd>i</kbd> Inventory</span><span><kbd>&lt; &gt;</kbd> Stairs</span>':saved.keys;if(active){if(latest)apply(latest);prompt();}else{dialog.close();onDemo();$('.location h1').textContent=saved.heading;controls.target.set(0,.1,0);camera.position.set(11,13,16);}onMode?.(active);}
 button.onclick=async()=>{if(active){setMode(false);return;}setMode(true);setPrompt('Starting isolated UnNetHack…');try{await post('/engine/start');connect();}catch(e){message(`Could not start engine: ${e.message}. Run npm run engine:build first.`);}};
 actions.querySelectorAll('[data-key]').forEach(b=>b.onclick=()=>{if(pending?.kind==='command')reply(Number(b.dataset.key));});
 addEventListener('keydown',e=>{if(!active||e.metaKey||e.ctrlKey||e.altKey)return;if(e.target instanceof HTMLInputElement)return;
   if(pending?.kind==='menu')return;let code;const directions={ArrowUp:107,ArrowDown:106,ArrowLeft:104,ArrowRight:108};
   if(pending?.kind==='command'){code=directions[e.key]??(e.key===' '?46:e.key.length===1?e.key.charCodeAt(0):undefined);}else code=directions[e.key]??(e.key==='Enter'?13:e.key==='Escape'?27:e.key.length===1?e.key.charCodeAt(0):undefined);
   if(code){e.preventDefault();e.stopImmediatePropagation();reply(code);}
 },true);
 return {get active(){return active;},update(t,dt){if(!active||!hero.target)return;const delta=hero.target.clone().sub(hero.g.position),moving=delta.length()>.025;if(moving)hero.g.rotation.y=Math.atan2(delta.x,delta.z);hero.g.position.lerp(hero.target,1-Math.exp(-dt*14));hero.body.position.y=Math.sin(t*(moving?18:2))*(moving?.035:.013);hero.legs.forEach((l,i)=>l.rotation.x=moving?Math.sin(t*18+i*Math.PI)*.5:0);hero.cape.rotation.x=-.17+Math.sin(t*3)*.06;if(hero.plume)hero.plume.rotation.z=-.16+Math.sin(t*2.4)*.035;
   const offset=hero.g.position.clone().sub(controls.target);offset.y=0;offset.multiplyScalar(1-Math.exp(-dt*3));controls.target.add(offset);camera.position.add(offset);lantern.position.copy(hero.g.position).add(new THREE.Vector3(0,3,0));
   for(const tile of tiles.values())if(tile.visible)tile.traverse(o=>o.userData.updateFire?.(t));
   updateTorchLights(t,dt);cavern.update(t,dt,hero.g.position);
   for(const a of actors.values()){let walking=false;if(a.target){const d=a.target.clone().sub(a.g.position);walking=d.length()>.025;if(walking)a.g.rotation.y=Math.atan2(d.x,d.z);a.g.position.lerp(a.target,1-Math.exp(-dt*10));if(a.legs)a.legs.forEach((l,i)=>l.rotation.x=walking?Math.sin(t*22+i*2)*.4:0);}if(a.tail){const tailRate=a.quirk==='dog'?7:a.quirk==='unicorn'?2.6:3;const tailSwing=a.quirk==='dog'?.34:a.quirk==='unicorn'?.16:.24;a.tail.rotation.z=Math.sin(t*tailRate)*tailSwing;}if(a.charm)a.charm.position.y=.3+Math.sin(t*4)*.025;if(a.body){const idle=a.quirk==='orc'?.025:a.quirk==='dragon'?.035:a.quirk==='unicorn'?.022:.015;a.body.position.y=Math.sin(t*(walking?22:2.5))*idle;}if(a.wings?.length)a.wings.forEach((wing,i)=>{if(a.quirk==='bat'){wing.rotation.z=(wing.userData.side||(i?1:-1))*Math.sin(t*14)*.65;}else if(a.quirk==='bee'){wing.rotation.y=(i?1:-1)*Math.sin(t*60)*.35;}else wing.rotation.y=(i?1:-1)*(-.18+Math.sin(t*5)*.12);});if((a.quirk==='hover'||a.quirk==='bat'||a.quirk==='bee')&&a.body)a.body.position.y=Math.sin(t*2.2+a.g.position.x)*.06;if(a.quirk==='dragon')a.g.rotation.z=Math.sin(t*1.7)*.025;if(a.quirk==='gridbug')a.g.rotation.z=Math.sin(t*9)*.035;if(a.quirk==='guard')a.g.rotation.z=Math.sin(t*1.3)*.012;const core=a.core||a.g.userData.core;if(core)core.material.emissiveIntensity=4.5+Math.sin(t*5)*1.4;}
   for(const item of groundItems.values()){if(!item.userData.coinPile)continue;item.userData.coinAge=(item.userData.coinAge||0)+dt;for(const coin of item.userData.coinPile){if(coin.settled||item.userData.coinAge<coin.delay)continue;coin.velocity-=9.8*dt;coin.disk.position.y+=coin.velocity*dt;coin.stamp.position.y+=coin.velocity*dt;if(coin.disk.position.y<=coin.target){coin.disk.position.y=coin.target;coin.stamp.position.y=coin.target+.019;coin.velocity*=-.16;if(Math.abs(coin.velocity)<.35)coin.settled=true;}}}
   for(const tile of tiles.values()){const liquid=tile.userData.liquid;if(!liquid)continue;const phase=tile.userData.liquidPhase||0;liquid.position.y=.01+Math.sin(t*2.2+phase)*.008;liquid.rotation.z=Math.sin(t*.8+phase)*.012;liquid.material.emissiveIntensity=.22+Math.sin(t*2.6+phase)*.06;}
   for(const w of wells.values())for(let i=0;i<w.children.length;i++){w.children[i].position.copy(wellTemplate.children[i].position);w.children[i].scale.copy(wellTemplate.children[i].scale);w.children[i].rotation.copy(wellTemplate.children[i].rotation);}
 }};
}
