#!/usr/bin/env python

import sys, os, re, math

hot_colors = {0:[0,0,255,255], 0.25:[0,255,255,255], 0.5:[0,255,0,255], 0.75:[255,255,0,255], 1:[255,0,0,255]}

# Va-Vb
def vecsub(Va,Vb):
    return [Va[0]-Vb[0],Va[1]-Vb[1],Va[2]-Vb[2]]

def vecmag(V):
    return math.sqrt(V[0]*V[0]+V[1]*V[1]+V[2]*V[2])

def vecdot(Va,Vb):
    return Va[0]*Vb[0]+Va[1]*Vb[1]+Va[2]*Vb[2]

def normalize(V):
    Vmag = vecmag(V)
    return [V[0]/Vmag, V[1]/Vmag, V[2]/Vmag]

def cross_prod(Va,Vb):
    return [Va[1]*Vb[2]-Va[2]*Vb[1],-(Va[0]*Vb[2]-Va[2]*Vb[0]),Va[0]*Vb[1]-Va[1]*Vb[0]]

def ray_tri_intersect(Pe, r, P1, P2, P3):
    V21 = vecsub(P2,P1)
    V32 = vecsub(P3,P2)
    Tn = cross_prod(V21,V32)
    tnum = Tn[0]*(P1[0]-Pe[0]) + Tn[1]*(P1[1]-Pe[1]) + Tn[2]*(P1[2]-Pe[2])
    tden = Tn[0]*r[0] + Tn[1]*r[1] + Tn[2]*r[2]
    if (tden == 0):
        return None
    t = tnum / tden
    # candidate point
    Pc = [Pe[0]+r[0]*t,Pe[1]+r[1]*t,Pe[2]+r[2]*t]

    # Now, check in/out of triangle using barycentric method
    V0 = vecsub(P3,P1)
    V1 = vecsub(P2,P1)
    V2 = vecsub(Pc,P1)
    dot00 = vecdot(V0,V0)
    dot01 = vecdot(V0,V1)
    dot02 = vecdot(V0,V2)
    dot11 = vecdot(V1,V1)
    dot12 = vecdot(V1,V2)
    invDenom = 1.0 / (dot00 * dot11 - dot01 * dot01)
    u = (dot11 * dot02 - dot01 * dot12) * invDenom
    v = (dot00 * dot12 - dot01 * dot02) * invDenom

    if u >= 0 and v >= 0 and (u+v) < 1:
        return Pc, u, v
    else:
        return None, 0, 0

def interp_field(u, v, A, B, C):
    return (1-u-v)*A + v*B + u*C

def interp_color(fval, fmin, fmax):
    ckeys = sorted(hot_colors.keys())
    cnum = len(ckeys)
    crange = ckeys[cnum-1] - ckeys[0]
    val = (fval-fmin)/(fmax-fmin) * crange + ckeys[0]
    if val <= ckeys[0]:
        return hot_colors[ckeys[0]]
    if val > ckeys[cnum-1]:
        return hot_colors[ckeys[cnum-1]]
    for i in range(1,len(ckeys)):
        if ckeys[i-1] <= val and val <= ckeys[i]:
            cval = (val-ckeys[i-1])/(ckeys[i]-ckeys[i-1])
            c0 = hot_colors[ckeys[i-1]]
            c1 = hot_colors[ckeys[i]]
            cdiff = vecsub(c1,c0)
            cdiff = [int(cdiff[0]*cval+c0[0]),int(cdiff[1]*cval+c0[1]),int(cdiff[2]*cval+c0[2])]
            return cdiff

def camera_params(LF, LA, VUP, FOV, W, H):
    N = [LA[0]-LF[0],LA[1]-LF[1],LA[2]-LF[2]]
    N = normalize(N)

    U = cross_prod(N,VUP)
    U = normalize(U)

    V = cross_prod(U,N)
    V = normalize(V)

    fova = math.tan(FOV/2)
    Xinc = [2*fova*U[0]/W, 2*fova*U[1]/W, 2*fova*U[2]/W]
    Yinc = [2*fova*V[0]/W, 2*fova*V[1]/W, 2*fova*V[2]/W]

    return N, U, V, Xinc, Yinc

def send_rays(LF, LA, VUP, FOV, W, H, P0, P1, P2, VMIN, VMAX):
    (N, U, V, Xinc, Yinc) = camera_params(LF, LA, VUP, FOV, W, H)

    for x in range(W):
        for y in range(H):
            R = [N[0] + 0.5*(2*y+1-H)*Yinc[0] + 0.5*(2*x+1-W)*Xinc[0],\
                 N[1] + 0.5*(2*y+1-H)*Yinc[1] + 0.5*(2*x+1-W)*Xinc[1],\
                 N[2] + 0.5*(2*y+1-H)*Yinc[2] + 0.5*(2*x+1-W)*Xinc[2]]
            R = normalize(R)
            ipoint_closests = None
            for i in range(12):
                (ipoint, u, v) = ray_tri_intersect(LF,R,P0[i],P1[i],P2[i])
                if ipoint != None:
                    if ipoint_closest != None:
                        if ipoint[2] > ipoint_closest[2]:
                            ipoint_closest = ipoint
                            fval = interp_field(u,v,P0[i][3],P1[i][3],P2[i][3])
                            rgb = interp_color(fval, VMIN, VMAX)
            print "%d:%d\t%d:%d:%d|%f"%(x,y,rgb[0],rgb[1],rgb[2],ipoint_closest[2])

def parse_zone_info(data):
    nodes = []
    nodeids = data[0].split(':')
    for n in nodeids:
        nspec = n.split('=')
        nodes.append(nspec[1])
    coords = []
    # data[1] = coords at each node
    coordlist = data[1].split(':')
    for coordval in coordlist:
        ctuple = coordval.split('$')
        coord = []
        for comp in ctuple:
            compval = comp.split('=')
            coord.append(compval[1])
        coords.append(coord)
    # data[2] = variables (both nodal and zonal)
    varlist = data[2].split(':')
    zonevars = {}
    nodevars = {}
    for varspec in varlist:
        vartuple = varspec.split('$')
        v = []
        if len(vartuple) == 1: # zone-centered case
            varvec = vartuple[0].split(';')
            if len(varvec) > 1: # vector var
                varval = varvec[0].split('=')
                varname = re.search("([^0-9]*)([0-9]{1})",varval[0])
                varname = varname.group(1)
                for varcomp in varvec:
                    varval = varcomp.split('=')
                    v.append(float(varval[1]))
            else: # scalar var
                varval = vartuple[0].split('=')
                varname = re.search("([^0-9]*)([0-9]{1})",varval[0])
                if varname == None:
                    continue
                varname = varname.group(1)
                v.append(float(varval[1]))
            zonevars[varname] = v
        else: # node-centered case
            for t in vartuple:
                varvec = t.split(';')
                if len(varvec) > 1: # vector var
                    varval = varvec[0].split('=')
                    varname = re.search("([^0-9]*)([0-9]{1})",varval[0])
                    varname = varname.group(1)
                    for varcomp in varvec:
                        varval = varcomp.split('=')
                        v.append(float(varval[1]))
                else: # scalar var
                    varval = varvec[0].split('=')
                    varname = re.search("([^0-9]*)([0-9]{1})",varval[0])
                    varname = varname.group(1)
                    v.append(float(varval[1]))
            nodevars[varname] = v
    return nodes, coords, zonevars, nodevars

def render_tri(coords, vals, na, nb, nc):
    p0 = [float(coords[na][0]),float(coords[na][1]),float(coords[na][2]),float(vals[na])]
    p1 = [float(coords[nb][0]),float(coords[nb][1]),float(coords[nb][2]),float(vals[nb])]
    p2 = [float(coords[nc][0]),float(coords[nc][1]),float(coords[nc][2]),float(vals[nc])]
    send_rays([0,0,10],[0,0,0],[0,1,0],0.5,20,20,p0,p1,p2,-10,10)

def render_zone(coords, vals):
    if len(coords) == 8: # hex
        render_tri(coords, vals, 0, 1, 5)
        render_tri(coords, vals, 0, 5, 4)

        render_tri(coords, vals, 0, 2, 1)
        render_tri(coords, vals, 0, 3, 2)

        render_tri(coords, vals, 0, 4, 7)
        render_tri(coords, vals, 0, 7, 3)
        
        render_tri(coords, vals, 1, 2, 6)
        render_tri(coords, vals, 1, 6, 5)

        render_tri(coords, vals, 2, 3, 7)
        render_tri(coords, vals, 2, 7, 6)

        render_tri(coords, vals, 4, 5, 6)
        render_tri(coords, vals, 4, 6, 7)

# input comes from STDIN (standard input)
for line in sys.stdin:

    # remove leading and trailing whitespace
    line = line.strip()

    # split the line into words
    (key, value) = line.split()

    data = value.split('|') # 1rst level split (nodes|coords|vars) sections

    (nodes, coords, zonevars, nodevars) = parse_zone_info(data)

    mode = os.environ['PLOTMODE']
    var = os.environ['PLOTVAR']

    if mode == "threshold":
        min = float(os.environ['THRESHOLD_MIN'])
        max = float(os.environ['THRESHOLD_MAX'])
        if var in nodevars:
            nodevals = nodevars[var]
            for val in nodevals:
                if val >= min and val <= max:
                    render_zone(coords, nodevals)
                    continue
        elif var in zonevars:
            val = zonevars[var]
            if val >= min and val <= max:
                render_zone(coords, val)
                continue
    elif mode == "iso-contour":
        isovals = os.eniron['ISOVALS'].split(':') 
