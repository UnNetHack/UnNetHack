// Conservative fallback until the bridge publishes structured melee events.
export function meleeDirection(key){
 return {h:[-1,0],j:[0,1],k:[0,-1],l:[1,0],y:[-1,-1],u:[1,-1],b:[-1,1],n:[1,1]}[String.fromCharCode(key).toLowerCase()]??null;
}
export function confirmsPlayerMelee(text){
 return /^You (?:hit|miss|smite|kill|destroy|strike|slash|stab)\b/.test(text);
}
export function poseMelee(actor,age){
 const weight=age>=0&&age<.35?Math.sin((age/.35)*Math.PI):0;
 if(actor.arm)actor.arm.rotation.x=-weight*1.9;
 if(actor.wrist)actor.wrist.rotation.x=weight*.34;
 if(actor.weaponSocket)actor.weaponSocket.rotation.z=-weight*1.18;
 if(actor.shieldArm)actor.shieldArm.rotation.z=weight*.12;
}
