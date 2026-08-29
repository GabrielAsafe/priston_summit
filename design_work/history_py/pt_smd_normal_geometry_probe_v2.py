#!/usr/bin/env python3
import argparse, math, struct, sys
from pathlib import Path

SMD_HEADER=b'SMD Model data Ver 0.62'
H=556; INFO=40; OBJ=2236; VERT=24; FACE=36; TEX=32; ROT=20; POS=16; SCALE=16; MAT=64
OFF_PHYS=16; OFF_NV=84; OFF_NF=88; OFF_NT=92; OFF_NAME=172; OFF_TM=240; OFF_NR=684; OFF_NP=688; OFF_NS=692

def i32(d,o): return struct.unpack_from('<i',d,o)[0]
def u32(d,o): return struct.unpack_from('<I',d,o)[0]
def cstr(b): return b.split(b'\0',1)[0].decode('latin1','replace')
def norm(a):
    s=math.sqrt(sum(x*x for x in a))
    return None if s<1e-20 else tuple(x/s for x in a)
def add(a,b): return tuple(a[i]+b[i] for i in range(3))
def sub(a,b): return tuple(a[i]-b[i] for i in range(3))
def mul(a,s): return tuple(x*s for x in a)
def dot(a,b): return sum(a[i]*b[i] for i in range(3))
def cross(a,b): return (a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0])
def angle(a,b):
    a=norm(a); b=norm(b)
    if a is None or b is None: return 0.0
    return math.acos(max(-1,min(1,dot(a,b))))
def angle_deg(a,b):
    a=norm(a); b=norm(b)
    if a is None or b is None: return None
    return math.degrees(math.acos(max(-1,min(1,dot(a,b)))))
def m(tm,r,c): return tm[(r-1)*4+c-1]

def parse_meta(d):
    if not d.startswith(SMD_HEADER): raise ValueError('bad SMD header')
    n=i32(d,24); infos=[]
    for k in range(n):
        o=H+k*INFO
        infos.append({'index':k,'name':cstr(d[o:o+32]),'length':i32(d,o+32),'start':i32(d,o+36)})
    return infos

def layout(start,nv,nf,nt,nr,np,ns,phys):
    vv=start+OBJ; ff=vv+VERT*nv; tt=ff+FACE*nf; rr=tt+TEX*nt; pp=rr+ROT*nr; ss=pp+POS*np; prev=ss+SCALE*ns
    phy=prev+MAT*nr if phys else None
    end=(phy+32*nv) if phys else (prev+MAT*nr)
    return vv,ff,tt,phy,end

def parse_obj(d,name):
    info=next(x for x in parse_meta(d) if x['name']==name)
    raw=d[info['start']:info['start']+OBJ]
    nv=i32(raw,OFF_NV); nf=i32(raw,OFF_NF); nt=i32(raw,OFF_NT); nr=i32(raw,OFF_NR); np=i32(raw,OFF_NP); ns=i32(raw,OFF_NS); phys=u32(raw,OFF_PHYS)!=0
    vv,ff,tt,phy,end=layout(info['start'],nv,nf,nt,nr,np,ns,phys)
    if end != info['start']+info['length']: raise ValueError(f'{name}: layout mismatch')
    verts=[struct.unpack_from('<6i',d,vv+i*VERT) for i in range(nv)]
    faces=[]
    for i in range(nf):
        a,b,c,matid=struct.unpack_from('<4H',d,ff+i*FACE)
        faces.append((a,b,c,matid))
    bones=[cstr(d[phy+i*32:phy+(i+1)*32]) for i in range(nv)] if phys else ['']*nv
    return {'nv':nv,'nf':nf,'verts':verts,'faces':faces,'bones':bones}

def parse_bones(d):
    out={}
    for info in parse_meta(d):
        raw=d[info['start']:info['start']+OBJ]
        name=cstr(raw[OFF_NAME:OFF_NAME+32]); out[name]=struct.unpack_from('<16i',raw,OFF_TM)
    return out

def world_pos(v,tm):
    x,y,z=v[:3]
    rx=x*m(tm,1,1)+y*m(tm,2,1)+z*m(tm,3,1)
    ry=x*m(tm,1,2)+y*m(tm,2,2)+z*m(tm,3,2)
    rz=x*m(tm,1,3)+y*m(tm,2,3)+z*m(tm,3,3)
    return ((rx>>8)+m(tm,4,1),(ry>>8)+m(tm,4,2),(rz>>8)+m(tm,4,3))

def normal_row(n,tm):
    x,y,z=n
    return norm((x*m(tm,1,1)+y*m(tm,2,1)+z*m(tm,3,1),x*m(tm,1,2)+y*m(tm,2,2)+z*m(tm,3,2),x*m(tm,1,3)+y*m(tm,2,3)+z*m(tm,3,3)))

def inv3(a):
    A,B,C=a[0]; D,E,F=a[1]; G,H,I=a[2]
    det=A*(E*I-F*H)-B*(D*I-F*G)+C*(D*H-E*G)
    if abs(det)<1e-20: raise ValueError('singular 3x3')
    return (( (E*I-F*H)/det,(C*H-B*I)/det,(B*F-C*E)/det),((F*G-D*I)/det,(A*I-C*G)/det,(C*D-A*F)/det),((D*H-E*G)/det,(B*G-A*H)/det,(A*E-B*D)/det))

def normal_invtrans(n,tm):
    R=((m(tm,1,1)/256,m(tm,1,2)/256,m(tm,1,3)/256),(m(tm,2,1)/256,m(tm,2,2)/256,m(tm,2,3)/256),(m(tm,3,1)/256,m(tm,3,2)/256,m(tm,3,3)/256))
    inv=inv3(R)
    # row-vector normal transform: n * inverse(R)^T
    x,y,z=n
    return norm((x*inv[0][0]+y*inv[0][1]+z*inv[0][2],x*inv[1][0]+y*inv[1][1]+z*inv[1][2],x*inv[2][0]+y*inv[2][1]+z*inv[2][2]))

def stats(xs):
    a=sorted(x for x in xs if x is not None)
    def q(p):
        t=(len(a)-1)*p; lo=int(math.floor(t)); hi=int(math.ceil(t)); f=t-lo
        return a[lo]*(1-f)+a[hi]*f
    return {'n':len(a),'mean':sum(a)/len(a),'p50':q(.5),'p95':q(.95),'max':a[-1]}
def sf(s): return f"n={s['n']} mean={s['mean']:.4f} p50={s['p50']:.4f} p95={s['p95']:.4f} max={s['max']:.4f} deg"

def probe(obj,bones):
    wp=[]; sn_row=[]; sn_inv=[]
    for v,b in zip(obj['verts'],obj['bones']):
        tm=bones[b]; wp.append(tuple(c/256.0 for c in world_pos(v,tm)))
        sn_row.append(normal_row(v[3:6],tm)); sn_inv.append(normal_invtrans(v[3:6],tm))
    area=[(0.,0.,0.) for _ in range(obj['nv'])]; equal=[(0.,0.,0.) for _ in range(obj['nv'])]; angw=[(0.,0.,0.) for _ in range(obj['nv'])]
    degenerate=0
    for a,b,c,_ in obj['faces']:
        pa,pb,pc=wp[a],wp[b],wp[c]
        cr=cross(sub(pb,pa),sub(pc,pa)); un=norm(cr)
        if un is None: degenerate+=1; continue
        for vi in (a,b,c):
            area[vi]=add(area[vi],cr); equal[vi]=add(equal[vi],un)
        aa=angle(sub(pb,pa),sub(pc,pa)); ab=angle(sub(pa,pb),sub(pc,pb)); ac=angle(sub(pa,pc),sub(pb,pc))
        angw[a]=add(angw[a],mul(un,aa)); angw[b]=add(angw[b],mul(un,ab)); angw[c]=add(angw[c],mul(un,ac))
    geoms={'AREA':list(map(norm,area)),'EQUAL':list(map(norm,equal)),'ANGLE':list(map(norm,angw))}
    serials={'BONE_ROW':sn_row,'BONE_INVTRANS':sn_inv}
    results=[]
    for sk,sv in serials.items():
      for gk,gv in geoms.items():
        for sign in (1,-1):
          errs=[]
          for x,y in zip(sv,gv):
            yy=y if sign==1 else tuple(-c for c in y) if y else None
            errs.append(angle_deg(x,yy))
          st=stats(errs); results.append((st['mean'],st['p95'],st['max'],f'{sk} vs {"+" if sign==1 else "-"}{gk}',st,errs))
    results.sort()
    return results,degenerate

def main():
    ap=argparse.ArgumentParser(); ap.add_argument('--smd',required=True); ap.add_argument('--skeleton',required=True); ap.add_argument('--object',action='append'); ap.add_argument('--top',type=int,default=10); a=ap.parse_args()
    sd=Path(a.smd).read_bytes(); bd=Path(a.skeleton).read_bytes(); bones=parse_bones(bd); names=a.object or ['MshC01','MsmC01','MslC01']
    print('PT_SMD_NORMAL_GEOMETRY_PROBE_V2')
    bests=[]
    for name in names:
        o=parse_obj(sd,name); results,degen=probe(o,bones)
        print(f'\nOBJECT {name} vertices={o["nv"]} faces={o["nf"]} degenerate_faces={degen}')
        for _,_,_,label,st,_ in results[:8]: print(f'  {label:30s} {sf(st)}')
        _,_,_,label,st,errs=results[0]; bests.append((name,label,st))
        print('BEST=',label)
        worst=sorted(((e,i,o['bones'][i]) for i,e in enumerate(errs) if e is not None),reverse=True)[:a.top]
        for e,i,b in worst: print(f"  WORST v={i} bone={b!r} angle={e:.4f}deg")
    print('\nSUMMARY')
    for n,l,s in bests: print(f'{n}: {l} mean={s["mean"]:.4f} p95={s["p95"]:.4f} max={s["max"]:.4f}')
    labels={x[1] for x in bests}; p95=max(x[2]['p95'] for x in bests); mx=max(x[2]['max'] for x in bests)
    if len(labels)==1 and p95<=5 and mx<=20:
        print('normal_geometry_probe_check=PASS')
    else:
        print('normal_geometry_probe_check=INCONCLUSIVE')

if __name__=='__main__': main()
