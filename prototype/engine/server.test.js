import test from 'node:test';
import assert from 'node:assert/strict';
import {encodeReply} from './server.js';
test('stale and duplicate requests cannot advance the game',()=>{assert.throws(()=>encodeReply({id:2,kind:'command'},{id:1,value:46}));assert.throws(()=>encodeReply(null,{id:2,value:46}));});
test('line replies cannot inject a second turn',()=>{assert.throws(()=>encodeReply({id:1,kind:'line'},{id:1,value:'foo\n46'}));assert.equal(encodeReply({id:1,kind:'line'},{id:1,value:'inventory'}),'inventory\n');});
test('keys and menu replies are bounded',()=>{assert.equal(encodeReply({id:1,kind:'command'},{id:1,value:46}),'46\n');assert.throws(()=>encodeReply({id:1,kind:'command'},{id:1,value:999}));assert.throws(()=>encodeReply({id:1,kind:'menu'},{id:1,value:'0\n1'}));assert.equal(encodeReply({id:1,kind:'menu'},{id:1,value:'0,2'}),'0,2\n');});
