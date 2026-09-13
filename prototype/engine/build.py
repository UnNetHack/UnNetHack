#!/usr/bin/env python3
"""Build an isolated engine. Never invokes make install or modifies the source build."""
from pathlib import Path
import os, shutil, subprocess, json, signal, hashlib, time
ROOT=Path(__file__).resolve().parents[2]
WORK=ROOT/'prototype'/'.engine'
SOURCE=WORK/'source'
PREFIX=WORK/'runtime'
WORK.mkdir(exist_ok=True)
assert WORK.resolve().is_relative_to((ROOT/'prototype').resolve())
# Refresh source inputs without resetting unchanged timestamps or generated configuration.
def ignore(directory,names):
    banned={'Makefile','.git','.claude','.agents','.codex','prototype','node_modules','output','tmp','frames','tilesets','autom4te.cache'}
    generated={'Makefile','autoconf.h','autoconf_paths.h','config.log','config.status','Sysunix'}
    relative=Path(directory).relative_to(ROOT)
    generated_paths={'include/date.h','include/onames.h','include/pm.h','include/vis_tab.h','src/monstr.c','src/tile.c','src/vis_tab.c','util/lev_lex.c','util/lev_yacc.c','util/dgn_lex.c','util/dgn_yacc.c'}
    generated.update(n for n in names if str(relative/n) in generated_paths)
    return [n for n in names if n in banned or n in generated or n.endswith(('.o','.d','.h-t')) or n in {'unnethack','recover','makedefs','lev_comp','dgn_comp','dlb'}]
def sync(src,dst):
    if not Path(dst).exists() or Path(src).read_bytes()!=Path(dst).read_bytes():
        shutil.copyfile(src,dst)
        shutil.copymode(src,dst)
    return str(dst)
shutil.copytree(ROOT,SOURCE,ignore=ignore,copy_function=sync,dirs_exist_ok=True)
sync(ROOT/'prototype/engine/bridge.c',SOURCE/'src/bridge.c')

def run(args,name,timeout):
    print(name,flush=True)
    with (WORK/(name+'.log')).open('w') as log:
        p=subprocess.Popen(args,cwd=SOURCE,stdout=log,stderr=subprocess.STDOUT,start_new_session=True)
        deadline=time.monotonic()+timeout
        try:
            while p.poll() is None:
                if time.monotonic()>deadline or Path(log.name).stat().st_size>8_000_000:
                    raise subprocess.TimeoutExpired(args,timeout)
                time.sleep(.1)
            code=p.returncode
        except subprocess.TimeoutExpired:
            os.killpg(p.pid,signal.SIGTERM)
            p.wait(timeout=10)
            raise RuntimeError(name+' exceeded its bounded timeout; see '+str(log.name))
    if code:
        print((WORK/(name+'.log')).read_text()[-7000:])
        raise SystemExit(code)
lua_c=subprocess.check_output(['pkg-config','--cflags','lua'],text=True).strip()
lua_l=subprocess.check_output(['pkg-config','--libs','lua'],text=True).strip()
signature=hashlib.sha256((str(PREFIX)+lua_c+lua_l+'bridge-build-v2').encode())
for f in [SOURCE/'configure',*sorted((SOURCE/'sys/autoconf').glob('Makefile.*'))]:signature.update(f.read_bytes())
stamp=WORK/'config-signature'
configured=stamp.exists() and stamp.read_text()==signature.hexdigest() and (SOURCE/'src/Makefile').exists()
if not configured:
    run(['./configure','--prefix='+str(PREFIX),'--with-owner='+os.environ.get('USER','dpalm'),'--with-group=staff','--enable-tty-graphics','--disable-sdl-graphics','--disable-curses-graphics','CFLAGS=-O2 -DBRIDGE_GRAPHICS','LUA_INCLUDE='+lua_c,'LUA_LIB='+lua_l],'configure',120)
    run(['make','-B','include/autoconf_paths.h'],'paths',20)
    with (SOURCE/'src/Makefile').open('a') as f:
        f.write('\n# Experimental bridge, isolated build only.\nWINOBJ += bridge.o\nSysunix: bridge.o\nbridge.o: bridge.c $(HACK_H)\n\t$(CC) $(CFLAGS) -c bridge.c\n')
    stamp.write_text(signature.hexdigest())
paths=(SOURCE/'include/autoconf_paths.h').read_text()
assert str(PREFIX) in paths and '/Users/dpalm/unnethack' not in paths, 'Unsafe compile-time paths'
# Keep the local rule present if a generated Makefile was refreshed.
makefile=SOURCE/'src/Makefile'
if 'WINOBJ += bridge.o' not in makefile.read_text():
    with makefile.open('a') as f:
        f.write('\n# Experimental bridge, isolated build only.\nWINOBJ += bridge.o\nSysunix: bridge.o\nbridge.o: bridge.c $(HACK_H)\n\t$(CC) $(CFLAGS) -c bridge.c\n')
# The legacy top-level data targets can race while rebuilding lev_comp.
run(['make','-j4','unnethack'],'build',420)
run(['make','-j1','all'],'data',180)
# Assemble runtime explicitly. No install target, permissions changes or live save copies.
share=PREFIX/'share/unnethack';var=PREFIX/'var/unnethack';home=PREFIX/'home'
for d in [share,var,home,var/'saves',var/'level',var/'bones']:
    d.mkdir(parents=True,exist_ok=True)
for f in (SOURCE/'dat').iterdir():
    if f.is_file() and (not (share/f.name).exists() or f.read_bytes()!=(share/f.name).read_bytes()):
        temporary=share/(f.name+'.next')
        shutil.copy2(f,temporary)
        os.replace(temporary,share/f.name)
for n in ['perm','record','logfile','xlogfile']:(var/n).touch(exist_ok=True)
# Atomic replacement leaves any running character on its existing executable inode.
next_binary=PREFIX/'unnethack.next'
shutil.copy2(SOURCE/'src/unnethack',next_binary)
if os.uname().sysname=='Darwin': subprocess.run(['codesign','--force','-s','-',str(next_binary)],check=True)
os.replace(next_binary,PREFIX/'unnethack')
(WORK/'manifest.json').write_text(json.dumps({'binary':str(PREFIX/'unnethack'),'prefix':str(PREFIX),'cwd':str(share),'home':str(home)}))
print('Isolated engine ready: '+str(PREFIX/'unnethack'))
