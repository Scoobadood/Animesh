//------------------------------------------------------------------
#version 330 core
//------------------------------------------------------------------
// Ray tracer ver: 1.000
//------------------------------------------------------------------
in smooth vec3      ray_pos;    // ray start position
in smooth vec3      ray_dir;    // ray start direction

uniform int         fac_siz;    // square texture x,y resolution size
uniform int         fac_num;    // number of valid floats in texture
uniform sampler2D   fac_txr;    // scene mesh data texture
out layout(location=0) vec4 frag_col;
//---------------------------------------------------------------------------
void main(void) {

    const vec3 back_col=vec3(0.2,0.2,0.2);      // background color

    const float _zero=1e-6;     // to avoid intrsection with start point of ray
    const int _fac_triangles=0; // r,g,b, refl,refr,n, type, triangle count, { x0,y0,z0,x1,y1,z1,x2,y2,z2 }
    const int _fac_spheres  =1; // r,g,b, refl,refr,n, type, sphere count,   { x,y,z,r }

    // ray scene intersection
    struct _ray {
        vec3 pos,dir,nor;
        vec3 col;
    };
    const int _lvls=5;
    const int _rays=(1<<_lvls)-1;
    _ray ray[_rays]; int rays;

    vec3 v0,v1,v2,pos;
    vec3 c,col;
    float refr,refl;
    float tt,t,n1,a;
    int i0,ii,num,id;

    // fac texture access
    vec2 st; int i,j; float ds=1.0/float(fac_siz-1);
    #define fac_get texture(fac_txr,st).r; st.s+=ds; i++; j++; if (j==fac_siz) { j=0; st.s=0.0; st.t+=ds; }
    // enque start ray
    ray[0].pos=ray_pos;
    ray[0].dir=normalize(ray_dir);
    ray[0].nor=vec3(0.0,0.0,0.0);
    ray[0].refl=0.0;
    ray[0].refr=0.0;
    ray[0].n0=n0;
    ray[0].n1=1.0;
    ray[0].l =0.0;
    ray[0].lvl=0;
    ray[0].i0=-1;
    ray[0].i1=-1;
    rays=1;


    // loop all enqued rays
    for (i0=0;i0<rays;i0++)
        {
        // loop through all objects
        // find closest forward intersection between them and ray[i0]
        // strore it to ray[i0].(nor,col)
        // strore it to pos,n1
        t=tt=-1.0; ii=1; ray[i0].l=0.0;
        ray[i0].col=back_col;
        pos=ray[i0].pos; n1=n0;
        for (st=vec2(0.0,0.0),i=j=0;i<fac_num;)
            {
            c.r=fac_get;            // RGBA
            c.g=fac_get;
            c.b=fac_get;
            refl=fac_get;
            refr=fac_get;
            n1=fac_get;             // refraction index
            a=fac_get; id=int(a);   // object type
            a=fac_get; num=int(a);  // face count

            if (id==_fac_triangles)
             for (;num>0;num--)
                {
                v0.x=fac_get; v0.y=fac_get; v0.z=fac_get;
                v1.x=fac_get; v1.y=fac_get; v1.z=fac_get;
                v2.x=fac_get; v2.y=fac_get; v2.z=fac_get;
                vec3 e1,e2,n,p,q,r;
                float t,u,v,det,idet;
                //compute ray triangle intersection
                e1=v1-v0;
                e2=v2-v0;
                // Calculate planes normal vector
                p=cross(ray[i0].dir,e2);
                det=dot(e1,p);
                // Ray is parallel to plane
                if (abs(det)<1e-8) continue;
                idet=1.0/det;
                r=ray[i0].pos-v0;
                u=dot(r,p)*idet;
                if ((u<0.0)||(u>1.0)) continue;
                q=cross(r,e1);
                v=dot(ray[i0].dir,q)*idet;
                if ((v<0.0)||(u+v>1.0)) continue;
                t=dot(e2,q)*idet;
                if ((t>_zero)&&((t<=tt)||(ii!=0)))
                    {
                    ii=0; tt=t;
                    // store color,n ...
                    ray[i0].col=c;
                    ray[i0].refl=refl;
                    ray[i0].refr=refr;
                    // barycentric interpolate position
                    t=1.0-u-v;
                    pos=(v0*t)+(v1*u)+(v2*v);
                    // compute normal (store as dir for now)
                    e1=v1-v0;
                    e2=v2-v1;
                    ray[i0].nor=cross(e1,e2);
                    }
                }

            if (id==_fac_spheres)
             for (;num>0;num--)
                {
                float r;
                v0.x=fac_get; v0.y=fac_get; v0.z=fac_get; r=fac_get;
                // compute l0 length of ray(p0,dp) to intersection with sphere(v0,r)
                // where rr= r^-2
                float aa,bb,cc,dd,l0,l1,rr;
                vec3 p0,dp;
                p0=ray[i0].pos-v0;  // set sphere center to (0,0,0)
                dp=ray[i0].dir;
                rr = 1.0/(r*r);
                aa=2.0*rr*dot(dp,dp);
                bb=2.0*rr*dot(p0,dp);
                cc=    rr*dot(p0,p0)-1.0;
                dd=((bb*bb)-(2.0*aa*cc));
                if (dd<0.0) continue;
                dd=sqrt(dd);
                l0=(-bb+dd)/aa;
                l1=(-bb-dd)/aa;
                if (l0<0.0) l0=l1;
                if (l1<0.0) l1=l0;
                t=min(l0,l1); if (t<=_zero) t=max(l0,l1);
                if ((t>_zero)&&((t<=tt)||(ii!=0)))
                    {
                    ii=0; tt=t;
                    // store color,n ...
                    ray[i0].col=c;
                    ray[i0].refl=refl;
                    ray[i0].refr=refr;
                    // position,normal
                    pos=ray[i0].pos+(ray[i0].dir*t);
                    ray[i0].nor=pos-v0;
                    }
                }
            }
        ray[i0].l=tt;
        ray[i0].nor=normalize(ray[i0].nor);
        // split ray from pos and ray[i0].nor
        if ((ii==0)&&(ray[i0].lvl<_lvls-1))
            {
            t=dot(ray[i0].dir,ray[i0].nor);

            }
        else if (i0>0) // ignore last ray if nothing hit
            {
            ray[i0]=ray[rays-1];
            rays--; i0--;
            }
        }

        col=ray[0].col;

    frag_col=vec4(col,1.0);
    }
//---------------------------------------------------------------------------
