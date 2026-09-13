export function groundNotice(request,lines){
 if(request?.kind!=='more'||!Array.isArray(lines))return null;
 const content=lines.map(line=>String(line).trim()).filter(Boolean);
 const header=content.findIndex(line=>/^Things that are here:$/.test(line));
 if(header<0)return null;
 return content.slice(header+1);
}
export function groundTile(frame){return frame?`${frame.branch}:${frame.depth}:${frame.player.x}:${frame.player.z}`:null;}
