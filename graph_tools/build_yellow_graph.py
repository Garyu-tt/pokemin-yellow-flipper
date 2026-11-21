#!/usr/bin/env python3
from pathlib import Path
from collections import Counter,defaultdict
from datetime import datetime,timezone
import sqlite3,json,gzip,re,subprocess
ROOT=Path('/home/mirbi/projects/pokemon-yellow-native')
UP=ROOT/'upstream/pokeyellow'
SRC=ROOT/'source'
OUT=ROOT/'graph/v1'
OUT.mkdir(parents=True,exist_ok=True)
DB=OUT/'pokemon_yellow_graph.sqlite3'
if DB.exists(): DB.unlink()
cx=sqlite3.connect(DB)
cx.executescript('''
CREATE TABLE nodes(id TEXT PRIMARY KEY,kind TEXT,label TEXT,source TEXT,line INTEGER,attrs TEXT);
CREATE TABLE edges(src TEXT,dst TEXT,kind TEXT,source TEXT,line INTEGER,attrs TEXT);
CREATE TABLE metadata(key TEXT PRIMARY KEY,value TEXT);
CREATE INDEX edges_src ON edges(src);CREATE INDEX edges_dst ON edges(dst);
CREATE INDEX edges_kind ON edges(kind);CREATE INDEX nodes_kind ON nodes(kind);
''')
nodes={}
edges=[]
def path(p):
    if p.is_relative_to(UP):return 'original/'+str(p.relative_to(UP))
    if p.is_relative_to(SRC):return 'flipper/'+str(p.relative_to(SRC))
    return str(p)
def node(id,kind,label=None,file=None,line=None,**attrs):
    if id not in nodes:
        nodes[id]=(id,kind,id if label is None else label,file,line,json.dumps(attrs,ensure_ascii=False,sort_keys=True))
    return id
def edge(a,b,kind,file=None,line=None,**attrs):
    if a not in nodes or b not in nodes:raise ValueError(('missing',a,b))
    edges.append((a,b,kind,file,line,json.dumps(attrs,ensure_ascii=False,sort_keys=True)))
def fnode(f):
    return node('file:'+path(f),'file',f.name,path(f))
def code(line):
    return re.split(r';(?=(?:[^"]*"[^"]*")*[^"]*$)',line,maxsplit=1)[0].strip()
def args(s):
    return [x.strip() for x in s.split(',')]
def snake(x):
    return re.sub(r'(?<!^)(?=[A-Z])','_',x).upper()
asmfiles=sorted(f for f in UP.rglob('*.asm') if '.git' not in f.parts)
cfiles=sorted([*SRC.glob('*.c'),*SRC.glob('*.h')])
for f in [*asmfiles,*cfiles]:fnode(f)
alllines={f:f.read_text(errors='replace').splitlines() for f in asmfiles}
labelpat=re.compile(r'^([A-Za-z_.][A-Za-z_0-9.#@]*)(::?)\s*(.*)$')
def label_match(s):
    if re.fullmatch(r'\.[A-Za-z_][A-Za-z_0-9.#@]*',s):s+=':'
    return labelpat.match(s)
syms=defaultdict(list)
label_at={}
locals={}
for f in asmfiles:
    parent=None
    for lineno,raw in enumerate(alllines[f],1):
        m=label_match(code(raw))
        if not m:continue
        name=m[1]
        if name.startswith('.'):qualified=(parent or path(f))+name
        else:parent=name;qualified=name
        id=node('asm:'+path(f)+':'+str(lineno),'asm_label',qualified,path(f),lineno,local=name.startswith('.'))
        edge('file:'+path(f),id,'defines',path(f),lineno)
        syms[qualified].append(id)
        label_at[(path(f),lineno)]=(id,parent)
        if name.startswith('.'):locals[(path(f),qualified)]=id
def resolve(name,f,parent):
    name=name.strip()
    if not re.fullmatch(r'[A-Za-z_.][A-Za-z_0-9.#@]*',name):return None
    if name.startswith('.'):return locals.get((path(f),(parent or path(f))+name))
    hit=syms.get(name,[])
    return hit[0] if len(hit)==1 else None
resolved=Counter();unresolved=Counter()
for f in asmfiles:
    active=None;parent=None
    for lineno,raw in enumerate(alllines[f],1):
        s=code(raw)
        if not s:continue
        m=label_match(s)
        if m:
            active,parent=label_at[(path(f),lineno)]
            s=m[3].strip()
            if not s:continue
        m=re.match(r'(?i)^(INCLUDE|INCBIN)\s+"([^"]+)"',s)
        if m:
            target=UP/m[2]
            edge(fnode(f),fnode(target),'includes' if m[1].upper()=='INCLUDE' else 'uses_asset',path(f),lineno,exists=target.exists())
        if active is None:continue
        m=re.match(r'(?i)^(call|jp|jr|farcall|farjp|predef|dw|dw_const)\s+(.+)$',s)
        if m:
            op=m[1].lower()
            target=args(m[2])[0] if op in ('dw','dw_const','farcall','farjp','predef') else args(m[2])[-1]
            kind={'call':'calls','jp':'jumps','jr':'branches','farcall':'calls','farjp':'jumps','predef':'calls','dw':'references','dw_const':'dispatches'}[op]
            dst=resolve(target,f,parent)
            if dst:edge(active,dst,kind,path(f),lineno,op=op);resolved[op]+=1
            elif re.fullmatch(r'[A-Za-z_.][A-Za-z_0-9.#@]*',target):unresolved[op]+=1
        m=re.match(r'(?i)^(SetEvent|SetEventReuseHL|ResetEvent|CheckEvent|CheckEventHL)\s+(EVENT_[A-Za-z0-9_]+)',s)
        if m:
            dst=node('event:'+m[2],'event',m[2])
            op=m[1].lower()
            kind='checks_event' if op.startswith('check') else 'resets_event' if op.startswith('reset') else 'sets_event'
            edge(active,dst,kind,path(f),lineno)
        m=re.match(r'(?i)^ld\s+(?:hl|de|bc)\s*,\s*([A-Za-z_.][\w.]*)',s)
        if m:
            dst=resolve(m[1],f,parent)
            if dst:edge(active,dst,'loads_address',path(f),lineno)
mapnames={}
for f in sorted((UP/'data/maps/headers').glob('*.asm')):
    for raw in alllines[f]:
        m=re.match(r'map_header\s+([A-Za-z0-9_]+)\s*,\s*([A-Z0-9_]+)',code(raw))
        if m:mapnames[m[1]]=m[2]
for f in sorted((UP/'data/maps/headers').glob('*.asm')):
    cur=None
    for lineno,raw in enumerate(alllines[f],1):
        s=code(raw)
        m=re.match(r'map_header\s+([A-Za-z0-9_]+),\s*([A-Z0-9_]+)',s)
        if m:
            cur=node('map:'+m[2],'map',m[1],path(f),lineno,map_constant=m[2])
            edge(fnode(f),cur,'declares_map',path(f),lineno)
        m=re.match(r'connection\s+(north|south|east|west),\s*([A-Za-z0-9_]+),\s*([A-Z0-9_]+),\s*(-?\d+)',s)
        if m and cur:
            dst=node('map:'+m[3],'map',m[2],path(f),lineno,map_constant=m[3])
            edge(cur,dst,'connects_to',path(f),lineno,direction=m[1],offset=int(m[4]))
for f in sorted((UP/'data/maps/objects').glob('*.asm')):
    lines=alllines[f]
    constants=[m[1] for raw in lines if (m:=re.match(r'def_warps_to\s+([A-Z0-9_]+)',code(raw)))]
    constant=constants[0] if constants else mapnames.get(f.stem,snake(f.stem))
    cur=node('map:'+constant,'map',f.stem,map_constant=constant)
    edge(fnode(f),cur,'describes_map',path(f))
    for lineno,raw in enumerate(lines,1):
        s=code(raw)
        m=re.match(r'warp_event\s+(\d+),\s*(\d+),\s*([A-Z0-9_]+),\s*(\d+)',s)
        if m:
            dst=node('map:'+m[3],'map',m[3],map_constant=m[3])
            edge(cur,dst,'warps_to',path(f),lineno,x=int(m[1]),y=int(m[2]),target_warp=int(m[4]))
        m=re.match(r'object_event\s+(-?\d+),\s*(-?\d+),\s*([A-Z0-9_]+),\s*([A-Z0-9_]+),\s*([A-Z0-9_]+),\s*([A-Z0-9_]+)',s)
        if m:
            id=node('npc:'+path(f)+':'+str(lineno),'npc',f.stem+' NPC '+str(lineno),path(f),lineno,x=int(m[1]),y=int(m[2]),sprite=m[3],movement=m[4],facing=m[5],text=m[6])
            edge(cur,id,'contains_npc',path(f),lineno)
        m=re.match(r'bg_event\s+(\d+),\s*(\d+),\s*([A-Z0-9_]+)',s)
        if m:
            id=node('bg:'+path(f)+':'+str(lineno),'background_event',m[3],path(f),lineno,x=int(m[1]),y=int(m[2]))
            edge(cur,id,'contains_bg_event',path(f),lineno)
f=UP/'data/moves/moves.asm'
for lineno,raw in enumerate(alllines[f],1):
    m=re.match(r'move\s+([A-Z0-9_]+),\s*([A-Z0-9_]+),\s*(\d+),\s*([A-Z0-9_]+),\s*(\d+),\s*(\d+)',code(raw))
    if m:
        id=node('move:'+m[1],'move',m[1],path(f),lineno,effect=m[2],power=int(m[3]),type=m[4],accuracy=int(m[5]),pp=int(m[6]))
        edge(fnode(f),id,'declares_move',path(f),lineno)
for f in sorted((UP/'data/pokemon/base_stats').glob('*.asm')):
    ls=alllines[f]
    ms=[(lineno,re.match(r'db\s+DEX_([A-Z0-9_]+)',code(raw))) for lineno,raw in enumerate(ls,1) if re.match(r'db\s+DEX_([A-Z0-9_]+)',code(raw))]
    if not ms:continue
    lineno,m=ms[0];species=m[1]
    id=node('pokemon:'+species,'pokemon',species,path(f),lineno)
    edge(fnode(f),id,'declares_pokemon',path(f),lineno)
    stats=None
    for raw in ls[lineno:]:
        s=code(raw)
        x=re.match(r'db\s+(\d+),\s*(\d+),\s*(\d+),\s*(\d+),\s*(\d+)$',s)
        if x and stats is None:
            stats=dict(zip(('hp','attack','defense','speed','special'),map(int,x.groups())))
        x=re.match(r'db\s+([A-Z]+),\s*([A-Z]+)$',s)
        if x and stats:
            row=list(nodes[id]);d=json.loads(row[-1]);d.update(stats);d.update(type1=x[1],type2=x[2]);row[-1]=json.dumps(d,sort_keys=True);nodes[id]=tuple(row);break
    for lineno,raw in enumerate(ls,1):
        if 'level 1 learnset' not in raw:continue
        m=re.match(r'db\s+(.+)',code(raw))
        if m:
            for name in args(m[1]):
                if name!='NO_MOVE':
                    mid=node('move:'+name,'move',name)
                    edge(id,mid,'starts_with_move',path(f),lineno)
f=UP/'data/pokemon/evos_moves.asm'
cur=None;phase=None
for lineno,raw in enumerate(alllines[f],1):
    s=code(raw)
    m=re.match(r'([A-Za-z0-9_]+)EvosMoves:$',s)
    if m:cur=node('pokemon:'+snake(m[1]),'pokemon',snake(m[1]),path(f),lineno,from_evos_moves=True);phase=None;continue
    if cur is None:continue
    if raw.lstrip().startswith('; Evolutions'):phase='evo';continue
    if raw.lstrip().startswith('; Learnset'):phase='learn';continue
    m=re.match(r'db\s+(.+)$',s)
    if not m:continue
    vals=args(m[1])
    if phase=='evo' and len(vals)>=3 and vals[0].startswith('EVOLVE_'):
        dst=node('pokemon:'+vals[-1],'pokemon',vals[-1])
        edge(cur,dst,'evolves_to',path(f),lineno,method=vals[0],requirements=vals[1:-1])
    if phase=='learn' and len(vals)==2 and vals[0].isdigit() and vals[1]!='0':
        dst=node('move:'+vals[1],'move',vals[1])
        edge(cur,dst,'learns_move_at_level',path(f),lineno,level=int(vals[0]))
funcpat=re.compile(r'(?m)^[ \t]*(?:(?:static|inline|extern|const|unsigned|signed)\s+)*[A-Za-z_][A-Za-z0-9_ \t\*]*?\s+([A-Za-z_]\w*)\s*\([^;{}]*\)\s*\{')
fnames=defaultdict(list)
csource={}
for f in cfiles:
    s=f.read_text(errors='replace');csource[f]=s
    for m in funcpat.finditer(s):
        name=m[1]
        if name in ('if','while','for','switch'):continue
        id=node('cfunc:'+path(f)+':'+name,'c_function',name,path(f),s.count('\n',0,m.start())+1)
        fnames[name].append(id);edge(fnode(f),id,'defines',path(f))
    for lineno,raw in enumerate(s.splitlines(),1):
        m=re.match(r'\s*#\s*include\s+"([^"]+)"',raw)
        if m:
            target=SRC/m[1]
            edge(fnode(f),fnode(target),'includes',path(f),lineno,exists=target.exists())
for f,s in csource.items():
    matches=list(funcpat.finditer(s))
    for i,m in enumerate(matches):
        id='cfunc:'+path(f)+':'+m[1]
        if id not in nodes:continue
        end=matches[i+1].start() if i+1<len(matches) else len(s)
        for target in set(re.findall(r'\b([A-Za-z_]\w*)\s*\(',s[m.end():end])):
            ids=fnames.get(target,[])
            if len(ids)==1:edge(id,ids[0],'native_calls',path(f))
for name,orig in {'YMapPallet':'PALLET_TOWN','YMapRedsHouse':'REDS_HOUSE_1F','YMapBluesHouse':'BLUES_HOUSE','YMapOaksLab':'OAKS_LAB','YMapRoute1':'ROUTE_1','YMapViridian':'VIRIDIAN_CITY','YMapViridianCenter':'VIRIDIAN_POKECENTER'}.items():
    target=node('map:'+orig,'map',orig,map_constant=orig)
    id=node('native-map:'+name,'native_map',name,path(SRC/'yellow_core.h'))
    edge(id,target,'represents_original_map',path(SRC/'yellow_core.h'))
    edge(fnode(SRC/'yellow_core.h'),id,'declares_native_map',path(SRC/'yellow_core.h'))
cx.executemany('INSERT INTO nodes VALUES (?,?,?,?,?,?)',sorted(nodes.values()))
cx.executemany('INSERT INTO edges VALUES (?,?,?,?,?,?)',edges)
git=subprocess.check_output(['git','-C',str(UP),'rev-parse','HEAD'],text=True).strip()
cx.executemany('INSERT INTO metadata VALUES (?,?)',[('schema','1'),('git_commit',git),('built_utc',datetime.now(timezone.utc).isoformat()),('type','static_non_executable')])
cx.commit()
assert cx.execute('PRAGMA quick_check').fetchone()[0]=='ok'
broken=cx.execute('SELECT count(*) FROM edges e LEFT JOIN nodes a ON e.src=a.id LEFT JOIN nodes b ON e.dst=b.id WHERE a.id IS NULL OR b.id IS NULL').fetchone()[0]
assert not broken
with gzip.open(OUT/'nodes.jsonl.gz','wt',encoding='utf-8') as g:
    for id,kind,label,source,line,attrs in cx.execute('SELECT * FROM nodes ORDER BY id'):
        g.write(json.dumps({'id':id,'kind':kind,'label':label,'source':source,'line':line,'attrs':json.loads(attrs)},ensure_ascii=False)+'\n')
with gzip.open(OUT/'edges.jsonl.gz','wt',encoding='utf-8') as g:
    for a,b,kind,source,line,attrs in cx.execute('SELECT * FROM edges ORDER BY src,dst,kind'):
        g.write(json.dumps({'src':a,'dst':b,'kind':kind,'source':source,'line':line,'attrs':json.loads(attrs)},ensure_ascii=False)+'\n')
world=list(cx.execute("SELECT DISTINCT a.label,b.label,e.kind FROM edges e JOIN nodes a ON e.src=a.id JOIN nodes b ON e.dst=b.id WHERE a.kind='map' AND b.kind='map' AND e.kind IN ('warps_to','connects_to') ORDER BY a.label,b.label,e.kind"))
def dotq(s):return '"'+s.replace('\\','\\\\').replace('"','\\"')+'"'
with (OUT/'world_map.dot').open('w') as g:
    g.write('digraph PokemonYellowWorld {\n  node [shape=box];\n')
    for a,b,kind in world:g.write('  '+dotq(a)+' -> '+dotq(b)+' [label='+dotq(kind)+'];\n')
    g.write('}\n')
report={'schema':1,'upstream_git':git,'asm_files':len(asmfiles),'native_c_and_h_files':len(cfiles),'nodes':len(nodes),'edges':len(edges),'nodes_by_kind':dict(cx.execute('SELECT kind,count(*) FROM nodes GROUP BY kind')),'edges_by_kind':dict(cx.execute('SELECT kind,count(*) FROM edges GROUP BY kind')),'resolved_refs':dict(resolved),'ambiguous_or_unresolved_refs':dict(unresolved),'unique_world_map_links':len(world),'quick_check':'ok','dangling_edges':broken,'limitations':['Static graph, not a runnable Game Boy or Tagma equivalent','Indirect calls, bank switching, CPU flags, timing, mutable state and macros are not fully resolved','ASM and C function references are heuristic','Learnset species naming may require manual normalization for special forms','Native-port status must be determined by separate tests']}
(OUT/'build_report.json').write_text(json.dumps(report,indent=2,ensure_ascii=False)+'\n')
(OUT/'README.md').write_text('Pokémon Yellow graph v1\n\nStatic source-grounded graph of upstream pret/pokeyellow and Flipper-native sources. Not executable.\n\nFiles: pokemon_yellow_graph.sqlite3 (indexed nodes,edges,metadata), nodes.jsonl.gz, edges.jsonl.gz, world_map.dot, build_report.json.\n\nExample: sqlite3 pokemon_yellow_graph.sqlite3 "SELECT kind,count(*) FROM edges GROUP BY kind;"\n\nRebuild: python3 /home/mirbi/projects/pokemon-yellow-native/graph_tools/build_yellow_graph.py\n\nNext: typed CFG/SSA, bank-switch resolution, event predicates, trace-based execution parity, lowering to Tagma/ARM.\n')
print(json.dumps(report,ensure_ascii=False,indent=2))
print('OUTPUT='+str(OUT))
