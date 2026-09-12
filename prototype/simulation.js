// Deliberately independent of UnNetHack: no file, save, or engine access.
export const fountain = { x: 0, z: -1 };
export function createState() { return { player: { x: -2, z: 2 }, cat: { x: -3, z: 2 }, enemy: { x: 3, z: -2, hp: 3 }, hp: 24, turn: 0, message: 'The fountain murmurs. Miso stays close.' }; }
export function isFloor(x,z) { return Number.isInteger(x) && Number.isInteger(z) && Math.abs(x)<=4 && Math.abs(z)<=3 && !(x===0 && z===-1); }
export function act(state, action) {
  if(state.hp<=0) return {type:'blocked'};
  if(action.type==='drink') {
    if(Math.abs(state.player.x-fountain.x)+Math.abs(state.player.z-fountain.z)>1){state.message='Step beside the spring to drink.';return {type:'blocked'};}
    state.hp=Math.min(24,state.hp+6); state.turn++;state.message='Cool water. A little strength returns.';return {type:'drink'};
  }
  if(action.type==='wait'){state.turn++;state.message='You listen to the water.';return {type:'wait'};}
  if(action.type!=='move' || Math.abs(action.dx)+Math.abs(action.dz)!==1) return {type:'blocked'};
  const x=state.player.x+action.dx,z=state.player.z+action.dz;
  if(!isFloor(x,z)){state.message='Ancient stone bars your way.';return {type:'blocked'};}
  if(state.enemy.hp>0 && x===state.enemy.x && z===state.enemy.z){
    state.enemy.hp--;state.turn++;
    if(state.enemy.hp>0){state.hp=Math.max(0,state.hp-3);state.message='Steel rings on stone. The sentinel strikes back.';}
    else state.message='The sentinel crumbles. The chamber is quiet again.';
    return {type:'attack'};
  }
  state.cat={...state.player};state.player={x,z};state.turn++;state.message='Your footsteps echo. Miso follows.';return {type:'move'};
}
