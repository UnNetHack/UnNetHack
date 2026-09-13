import {spawn} from 'node:child_process';
import {readFileSync,realpathSync} from 'node:fs';
import {fileURLToPath} from 'node:url';
import {resolve,sep} from 'node:path';
import {randomBytes} from 'node:crypto';

export function encodeReply(request, body) {
  if(!request || body.id!==request.id) throw new Error('Stale input; wait for the next engine prompt.');
  if(request.kind==='line') {
    if(typeof body.value!=='string'||body.value.length>200||/[\r\n\0]/.test(body.value))throw new Error('Invalid line');
    return body.value+'\n';
  }
  if(request.kind==='menu') {
    if(typeof body.value!=='string'||!/^(!|\d+(,\d+)*|)$/.test(body.value)||body.value.length>200)throw new Error('Invalid selection');
    return body.value+'\n';
  }
  if(!Number.isInteger(body.value)||body.value<1||body.value>255)throw new Error('Invalid key');
  return body.value+'\n';
}
export default function enginePlugin(){
 const root=fileURLToPath(new URL('../.engine/',import.meta.url));
 const token=randomBytes(24).toString('hex');
 let child=null,pending=null,frame=null,menu=null,text=null,ended=null,buffer='';const clients=new Set();const history=[];
 function send(event){if(event.type==='frame')frame=event;else if(event.type==='request')pending=event;else if(event.type==='menu')menu=event;else if(event.type==='text')text=event;else if(event.type==='ended')ended=event;else if(event.type==='message'||event.type==='status'){history.push(event);if(history.length>40)history.shift();}for(const res of clients)res.write(`data: ${JSON.stringify(event)}\n\n`);}
 function start(){
   if(child)return;
   const manifest=JSON.parse(readFileSync(resolve(root,'manifest.json'),'utf8'));
   for(const p of [manifest.binary,manifest.cwd,manifest.home,manifest.prefix])if(!realpathSync(p).startsWith(realpathSync(root)+sep))throw new Error('Engine path is outside isolated runtime');
   pending=frame=menu=text=ended=null;history.length=0;buffer='';
   child=spawn(manifest.binary,['-d',manifest.cwd,'-u','Wanderer','-p','Valkyrie','-r','human'],{cwd:manifest.cwd,env:{PATH:process.env.PATH,HOME:manifest.home,USER:process.env.USER,LOGNAME:process.env.LOGNAME,TERM:'dumb',NETHACKOPTIONS:'windowtype:bridge,name:Wanderer,role:Valkyrie,race:human,gender:female,align:lawful,pettype:cat,!news,!autopickup'},stdio:['pipe','pipe','pipe']});
   child.stdout.setEncoding('utf8');child.stdout.on('data',chunk=>{buffer+=chunk;if(buffer.length>2000000){send({type:'message',text:'Engine output exceeded protocol limit.'});child.stdin.end();buffer='';return;}let i;while((i=buffer.indexOf('\n'))>=0){const line=buffer.slice(0,i);buffer=buffer.slice(i+1);if(!line.trim())continue;try{const data=JSON.parse(line);if(['frame','request','menu','text','message','status','ended'].includes(data.type))send(data);}catch{send({type:'message',text:line.slice(0,500)});}}});
   child.stderr.on('data',b=>send({type:'message',text:String(b).slice(0,1000)}));
   child.on('error',e=>send({type:'message',text:e.message}));
   child.on('close',code=>{child=null;pending=null;send({type:'ended',text:`Engine session closed (${code}). Start again to resume any saved character.`});});
 }
 return {name:'deanhack-engine',configureServer(server){
   server.httpServer?.on('close',()=>{child?.stdin.end();for(const c of clients)c.end();});
   server.middlewares.use(async(req,res,next)=>{
     const path=req.url?.split('?')[0];if(!path?.startsWith('/engine/'))return next();
     const host=req.headers.host;
     // Vite may be opened as either localhost or 127.0.0.1. Both resolve to the
     // local machine; rejecting localhost makes the live controls look frozen.
     if(!host||! /^(?:127\.0\.0\.1|localhost):\d+$/.test(host)||(req.headers.origin&&req.headers.origin!==`http://${host}`)){res.writeHead(403);res.end('Local same-origin requests only');return;}
     const json=(code,data)=>{res.writeHead(code,{'Content-Type':'application/json','Cache-Control':'no-store'});res.end(JSON.stringify(data));};
     if(req.method==='GET'&&path==='/engine/token')return json(200,{token});
     if(req.method==='GET'&&path==='/engine/events'){
       res.writeHead(200,{'Content-Type':'text/event-stream','Cache-Control':'no-cache','Connection':'keep-alive'});clients.add(res);res.write(': connected\n\n');
       for(const event of [...history,frame,menu,text,ended,pending].filter(Boolean))res.write(`data: ${JSON.stringify(event)}\n\n`);
       const timer=setInterval(()=>res.write(': alive\n\n'),15000);req.on('close',()=>{clearInterval(timer);clients.delete(res);});return;
     }
     if(req.method!=='POST'||req.headers['x-engine-token']!==token)return json(403,{error:'Invalid local session token'});
     try{
       let body='';for await(const b of req){body+=b;if(body.length>4096)throw new Error('Request too large');}
       if(path==='/engine/start'){start();return json(200,{ok:true});}
       if(path==='/engine/input'){
         if(!child)throw new Error('Start the engine first');
         const reply=encodeReply(pending,JSON.parse(body));pending=null;menu=null;text=null;child.stdin.write(reply);return json(200,{ok:true});
       }
       return json(404,{error:'Unknown action'});
     }catch(e){return json(400,{error:e.message});}
   });
 }};
}
