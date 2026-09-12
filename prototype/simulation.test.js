import test from 'node:test';
import assert from 'node:assert/strict';
import {createState,act} from './simulation.js';
test('walls and fountain do not spend turns',()=>{const s=createState();s.player={x:0,z:0};act(s,{type:'move',dx:0,dz:-1});assert.equal(s.turn,0);s.player={x:4,z:0};act(s,{type:'move',dx:1,dz:0});assert.equal(s.turn,0);});
test('pet follows previous square',()=>{const s=createState();const old={...s.player};act(s,{type:'move',dx:1,dz:0});assert.deepEqual(s.cat,old);assert.equal(s.turn,1);});
test('combat resolves before entry and dead enemy permits movement',()=>{const s=createState();s.player={x:2,z:-2};for(let i=0;i<3;i++)act(s,{type:'move',dx:1,dz:0});assert.equal(s.enemy.hp,0);assert.equal(s.hp,18);assert.equal(s.player.x,2);act(s,{type:'move',dx:1,dz:0});assert.equal(s.player.x,3);});
test('drink requires proximity and caps health',()=>{const s=createState();s.hp=22;act(s,{type:'drink'});assert.equal(s.hp,22);assert.equal(s.turn,0);s.player={x:0,z:0};act(s,{type:'drink'});assert.equal(s.hp,24);assert.equal(s.turn,1);});
