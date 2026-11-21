#!/usr/bin/env python3
from pathlib import Path
import sqlite3,json
root=Path('/home/mirbi/projects/pokemon-yellow-native')
db=root/'graph/v1/pokemon_yellow_graph.sqlite3'
cx=sqlite3.connect(db)
nodes=[{'id':row[0],'name':row[1],'npc':row[2]} for row in cx.execute("SELECT id,label,(SELECT COUNT(*) FROM edges WHERE src=n.id AND kind='contains_npc') FROM nodes n WHERE kind='map' ORDER BY label")]
links=[{'a':row[0],'b':row[1],'type':row[2]} for row in cx.execute("SELECT src,dst,kind FROM edges WHERE kind IN ('warps_to','connects_to') AND src LIKE 'map:%' AND dst LIKE 'map:%'")]
data=json.dumps({'nodes':nodes,'links':links},ensure_ascii=False)
html=r"""<!doctype html>
<html lang="ru"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Pokémon Yellow — граф мира</title><style>
*{box-sizing:border-box}html,body{margin:0;background:#09141d;color:#e5f2ff;font:14px system-ui;height:100%;overflow:hidden}
header{display:flex;align-items:center;gap:12px;flex-wrap:wrap;padding:10px 14px;background:#102536;position:relative;z-index:2}
header strong{font-size:17px}input{background:#081a28;color:#fff;border:1px solid #416475;border-radius:9px;padding:9px;min-width:170px;flex:1;outline:none}
button{background:#1f5267;border:0;border-radius:8px;color:white;padding:9px 13px;cursor:pointer}
small{color:#9ab9c9}
#view{display:block;width:100%;height:calc(100dvh - 65px);touch-action:none}
#info{position:absolute;bottom:16px;left:16px;width:min(355px,calc(100vw - 32px));padding:14px;background:#102536ec;border:1px solid #386277;border-radius:12px;max-height:39dvh;overflow:auto}
#info b{font-size:17px}#info ul{padding-left:18px;margin:9px 0}#info li{padding:2px 0;cursor:pointer}
@media(max-width:600px){header{gap:6px}header strong{font-size:14px}small{display:none}#view{height:calc(100dvh - 95px)}#info{max-height:34dvh}}
</style></head><body>
<header><strong>Pokémon Yellow · World Graph</strong><input id="search" placeholder="Найти локацию, например PalletTown"><button id="reset">Сбросить вид</button><small>Колесо — масштаб · Перетаскивание — панорама · Нажатие — связи</small></header>
<canvas id="view"></canvas><div id="info"></div>
<script>
const G=__MAP_DATA__;
const nodes=G.nodes.map((n,i)=>({...n,x:Math.cos(i*2.39996)*Math.sqrt(i+1)*13,y:Math.sin(i*2.39996)*Math.sqrt(i+1)*13}));
const byId=new Map(nodes.map((n,i)=>[n.id,i]));
const links=G.links.filter(e=>byId.has(e.a)&&byId.has(e.b)).map(e=>({...e,i:byId.get(e.a),j:byId.get(e.b)}));
for(let step=0;step<210;step++){
 const fx=new Float64Array(nodes.length),fy=new Float64Array(nodes.length);
 for(let i=0;i<nodes.length;i++)for(let j=i+1;j<nodes.length;j++){
  const a=nodes[i],b=nodes[j],dx=a.x-b.x,dy=a.y-b.y,d=dx*dx+dy*dy+100;
  const v=800/d;fx[i]+=dx*v;fy[i]+=dy*v;fx[j]-=dx*v;fy[j]-=dy*v;
 }
 for(const e of links){
  const a=nodes[e.i],b=nodes[e.j],dx=b.x-a.x,dy=b.y-a.y,d=Math.hypot(dx,dy)+0.01;
  const v=(d-75)*0.006;fx[e.i]+=dx/d*v;fy[e.i]+=dy/d*v;fx[e.j]-=dx/d*v;fy[e.j]-=dy/d*v;
 }
 for(let i=0;i<nodes.length;i++){nodes[i].x+=Math.max(-6,Math.min(6,fx[i]))*.5;nodes[i].y+=Math.max(-6,Math.min(6,fy[i]))*.5;}
}
const cvs=document.getElementById('view'),ctx=cvs.getContext('2d'),search=document.getElementById('search'),panel=document.getElementById('info');
let scale=1,panX=0,panY=0,selected=byId.get('map:PALLET_TOWN')??0;
let W=0,H=0,drag=false,moved=false,startX=0,startY=0,lastX=0,lastY=0;
function neighbors(){const a=new Set([selected]);for(const e of links)if(e.i===selected)a.add(e.j);else if(e.j===selected)a.add(e.i);return a;}
function screen(n){return [W/2+panX+n.x*scale,H/2+panY+n.y*scale];}
function fit(){const xs=nodes.map(n=>n.x),ys=nodes.map(n=>n.y),dx=Math.max(...xs)-Math.min(...xs),dy=Math.max(...ys)-Math.min(...ys);scale=Math.min(W/(dx+170),H/(dy+140));panX=panY=0;render();}
function resize(){W=cvs.clientWidth;H=cvs.clientHeight;const dpr=window.devicePixelRatio||1;cvs.width=Math.round(W*dpr);cvs.height=Math.round(H*dpr);ctx.setTransform(dpr,0,0,dpr,0,0);fit();}
function drawInfo(){
 const n=nodes[selected],arr=[];
 for(const e of links)if(e.i===selected||e.j===selected){const other=nodes[e.i===selected?e.j:e.i];arr.push({other,type:e.type,out:e.i===selected});}
 arr.sort((a,b)=>a.other.name.localeCompare(b.other.name));
 panel.innerHTML='<b>'+n.name+'</b><div style="color:#aac0ca">NPC: '+n.npc+' · связей: '+arr.length+'</div><ul>'+arr.map(x=>'<li data-id="'+x.other.id+'">'+(x.out?'→ ':'← ')+x.other.name+' <small>('+x.type+')</small></li>').join('')+'</ul>';
 panel.querySelectorAll('li').forEach(el=>el.addEventListener('click',()=>choose(byId.get(el.dataset.id))));
}
function render(){
 ctx.clearRect(0,0,W,H);ctx.fillStyle='#09141d';ctx.fillRect(0,0,W,H);
 const visible=neighbors();
 for(const e of links){
  const [x1,y1]=screen(nodes[e.i]),[x2,y2]=screen(nodes[e.j]);const focused=e.i===selected||e.j===selected;
  ctx.strokeStyle=focused?(e.type==='warps_to'?'#62c8ee':'#ecc85e'):'#254151';
  ctx.lineWidth=focused?1.8:0.6;ctx.beginPath();ctx.moveTo(x1,y1);ctx.lineTo(x2,y2);ctx.stroke();
 }
 for(let i=0;i<nodes.length;i++){
  const n=nodes[i],p=screen(n),focus=i===selected,near=visible.has(i);
  ctx.beginPath();ctx.fillStyle=focus?'#ffd25a':near?'#68d6df':'#3c667b';ctx.arc(p[0],p[1],focus?7:near?5:2.7,0,Math.PI*2);ctx.fill();
  if(focus||near||scale>1.6){ctx.fillStyle=focus?'#ffdf75':'#d8e7ef';ctx.font=(focus?'bold ':'')+'11px system-ui';ctx.fillText(n.name,p[0]+8,p[1]-7);}
 }
 drawInfo();
}
function choose(i){if(i==null)return;selected=i;const [x,y]=screen(nodes[i]);panX+=W/2-x;panY+=H/2-y;render();}
cvs.addEventListener('pointerdown',e=>{drag=true;moved=false;startX=lastX=e.clientX;startY=lastY=e.clientY;cvs.setPointerCapture(e.pointerId)});
cvs.addEventListener('pointermove',e=>{if(!drag)return;let dx=e.clientX-lastX,dy=e.clientY-lastY;if(Math.hypot(e.clientX-startX,e.clientY-startY)>6)moved=true;panX+=dx;panY+=dy;lastX=e.clientX;lastY=e.clientY;render()});
cvs.addEventListener('pointerup',e=>{drag=false;if(moved)return;const box=cvs.getBoundingClientRect();let best=-1,dist=225;nodes.forEach((n,i)=>{const [x,y]=screen(n),d=(x-e.clientX+box.left)**2+(y-e.clientY+box.top)**2;if(d<dist){best=i;dist=d}});if(best>=0)choose(best)});
cvs.addEventListener('wheel',e=>{e.preventDefault();const d=e.deltaY<0?1.12:1/1.12;scale=Math.max(.08,Math.min(10,scale*d));render()},{passive:false});
search.addEventListener('input',e=>{const q=e.target.value.trim().toLowerCase();if(!q)return;const i=nodes.findIndex(n=>n.name.toLowerCase().includes(q)||n.id.toLowerCase().includes(q));if(i>=0)choose(i)});
document.getElementById('reset').onclick=fit;
window.addEventListener('resize',resize);resize();
</script></body></html>"""
path=root/'graph/v1/world_map.html'
path.write_text(html.replace('__MAP_DATA__',data),encoding='utf-8')
print('world_map.html',path.stat().st_size,'bytes',len(nodes),'maps',len(links),'edges')
