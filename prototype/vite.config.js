import {defineConfig} from 'vite';
import enginePlugin from './engine/server.js';
export default defineConfig({plugins:[enginePlugin()],server:{host:'127.0.0.1'}});
