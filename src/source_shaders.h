
#if !RNY_VERSION
    /*-----------*/ _SOURCE_SHADER("_glsl_version.h.glsl", { R"_SHAD(
    #version 330 // version 3.3
    #define ATTRIB(LOC)	layout(location=LOC)	in
    
    /*----------------------------------------------------*/ )_SHAD" } )
#else
    /*-----------*/ _SOURCE_SHADER("_glsl_version.h.glsl", { R"_SHAD(
    #version 150 // version 3.2
    #define ATTRIB(LOC)	attribute				
    
    /*----------------------------------------------------*/ )_SHAD" } )
#endif

/*-----------*/ _SOURCE_SHADER("_global_include.h.glsl", { R"_SHAD(

#define flt		float
#define si		int
#define ui		uint

#define v2		vec2
#define v3		vec3
#define v4		vec4
#define sv2		ivec2
#define sv3		ivec3
#define sv4		ivec4
#define uv2		uvec2
#define uv3		uvec3
#define uv4		uvec4

#define m2		mat2
#define m3		mat3
#define m4		mat4

#define PI			3.1415926535897932384626433832795
#define SQRT_2		1.4142135623730950488016887242097

#define DEG_360		(2 * PI)
#define DEG_180		(PI)
#define DEG_90		(0.5 * PI)

#define MAX_LIGHTS_COUNT 8

                        struct Transforms {
                                m4					model_to_cam;
                                m3					normal_model_to_cam; // only rotation, no scaling, to transform direction vectors
};

                        struct Material {
                                //// Albedo
                                // diffuse 'color' for dielectrics (reflections are always white (F0 has x=y=z))
                                // specular 'color' for conductors (no diffuse on metal)
                                v3					albedo;
                                //// Metallic
                                // 0: material is dielectric  1: material is conductor
                                // results get blended to simulate heterogenous or layered materials
                                flt					metallic;
                                
                                flt					roughness;
                                
                                flt					IOR;
};

                        struct Light {
                                                    // if w == 0.0:	xyz: direction to directional light source
                                                    // else:		xyz: pos of point light
                                v4					light_vec_cam;
                                v3					power;
                                ui					shad_i;
                                m4					cam_to_light;
};

layout(std140)			uniform Global {
                                m4					g_cam_to_clip;
                                
                                sv2					g_target_res;
                                v2					g_mouse_cursor_pos;
                                
                                Transforms			g_transforms;
                                
                                Material			g_mat;
                                
                                ui					g_lights_count;
                                ui					g_lightbulb_indx;
                                
                                Light				g_lights[MAX_LIGHTS_COUNT];
                                
                                // for debugging
                                m3					g_camera_to_world;
                                
                                // PBR dev
                                flt					g_luminance_prefilter_mip_count;
                                ui					g_luminance_prefilter_mip_indx;
                                flt					g_luminance_prefilter_roughness;
                                ui					g_luminance_prefilter_base_samples;
                                ui					g_source_cubemap_res;
                                
                                flt					g_avg_log_luminance;
                                
                                flt					g_shadow1_inv_far;
};
/*----------------------------------------------------*/ )_SHAD" } )

/*-----------*/ _SOURCE_SHADER("_global_include.h.vert", { R"_SHAD(
$include "_global_include.h.glsl"
/*----------------------------------------------------*/ )_SHAD" } )

/*-----------*/ _SOURCE_SHADER("_global_include.h.frag", { R"_SHAD(
$include "_global_include.h.glsl"

                    out	v4	fs_out_frag_col;

//#if DBG
v4		dbg_col =		v4(0);
bool	dbg_written	=	false;

void DBG_COL (in v4 col) {
    dbg_col = col;
    dbg_written = true;
}
void DBG_COL (in v3 col) {
    DBG_COL(v4(col, 1));
}
void DBG_COL (in flt col) {
    DBG_COL(v3(col));
}

#define FRAG_DONE(col) \
    fs_out_frag_col = col; \
    if (dbg_written) { fs_out_frag_col = dbg_col; } \
    return;

//#else
//void FRAG_DONE(col) fs_out_frag_col = col;
//#endif

v2 get_screen_pos () {
    return v2(gl_FragCoord) / v2(g_target_res);
}
v2 get_mouse_pos () {
    return g_mouse_cursor_pos;
}

bool dbg_left () {
    return ( get_screen_pos().x -get_mouse_pos().x ) <= 0.0;
}
bool dbg_right () {
    return ( get_screen_pos().x -get_mouse_pos().x ) > 0.0;
}
bool dbg_bottom () {
    return ( get_screen_pos().y -get_mouse_pos().y ) <= 0.0;
}
bool dbg_top () {
    return ( get_screen_pos().y -get_mouse_pos().y ) > 0.0;
}

#define DBG_LEFT	if (dbg_left())
#define DBG_RIGHT	if (dbg_right())
#define DBG_BOTTOM	if (dbg_bottom())
#define DBG_TOP		if (dbg_top())

bool in_rect (v2 v, v2 ll, v2 ur) {
    bvec2	a = lessThanEqual(ll, v);
    bvec2	b = greaterThanEqual(ur, v);
    return	all(a) && all(b);
}

// linearly map a range, zero
flt map (flt val, flt zero, flt one) {
    return clamp((val -zero) / (one -zero), 0.0, 1.0);
}
v2 map (v2 val, v2 zero, v2 one) {
    return clamp((val -zero) / (one -zero), v2(0), v2(1));
}
v3 map (v3 val, v3 zero, v3 one) {
    return clamp((val -zero) / (one -zero), v3(0), v3(1));
}
v4 map (v4 val, v4 zero, v4 one) {
    return clamp((val -zero) / (one -zero), v4(0), v4(1));
}
v2 map (v2 val, flt zero, flt one) {
    return clamp((val -v2(zero)) / (v2(one) -v2(zero)), v2(0), v2(1));
}
v3 map (v3 val, flt zero, flt one) {
    return clamp((val -v3(zero)) / (v3(one) -v3(zero)), v3(0), v3(1));
}
v4 map (v4 val, flt zero, flt one) {
    return clamp((val -v4(zero)) / (v4(one) -v4(zero)), v4(0), v4(1));
}

flt avg_xyz (v3 v) {
    return (v.x +v.y +v.z) / 3.0;
}

/*----------------------------------------------------*/ )_SHAD" } )

//////
// Vertex shaders
//////
/*-----------*/ _SOURCE_SHADER("_shadow_map_directional.vert", { R"_SHAD(
$include "_glsl_version.h.glsl"
$include "_global_include.h.vert"

ATTRIB(0)	v3		attrib_pos_model;

void main () {
    v4 pos_light = g_transforms.model_to_cam * v4(attrib_pos_model, 1); // model_to_light in this case
    v4 pos_clip = g_cam_to_clip * pos_light;
    
    gl_Position = pos_clip;
}
/*----------------------------------------------------*/ )_SHAD" } )

/*-----------*/ _SOURCE_SHADER("_shadow_map_omnidir.vert", { R"_SHAD(
$include "_glsl_version.h.glsl"
$include "_global_include.h.vert"

ATTRIB(0)	v3		attrib_pos_model;

out Vs_Out {
    smooth	v3		vs_interp_pos_light;
};

void main () {
    v4 pos_light = g_transforms.model_to_cam * v4(attrib_pos_model, 1); // model_to_light in this case
    
    v4 pos_clip = g_cam_to_clip * pos_light;
    
    gl_Position =					pos_clip;
    vs_interp_pos_light =			pos_light.xyz;
}
/*----------------------------------------------------*/ )_SHAD" } )

/*-----------*/ _SOURCE_SHADER("_model_pos.vert", { R"_SHAD(
$include "_glsl_version.h.glsl"
$include "_global_include.h.vert"

ATTRIB(0)	v3		attrib_pos_model;

void main () {
    v4 pos_cam = g_transforms.model_to_cam * v4(attrib_pos_model, 1);
    v4 pos_clip = g_cam_to_clip * pos_cam;
    
    gl_Position = pos_clip;
}
/*----------------------------------------------------*/ )_SHAD" } )

/*-----------*/ _SOURCE_SHADER("_sky_transform.vert", { R"_SHAD(
$include "_glsl_version.h.glsl"
$include "_global_include.h.vert"

ATTRIB(0)	v3		attrib_pos_model;

void main () {
    v3 pos_cam = m3(g_transforms.model_to_cam) * attrib_pos_model;
    
    v4 pos_clip = g_cam_to_clip * v4(pos_cam, 0);
    
    pos_clip.x = pos_cam.x * g_cam_to_clip[0][0];
    pos_clip.y = pos_cam.y * g_cam_to_clip[1][1];
    pos_clip.z = -(pos_cam.z * 2.0); // after the persp divide the depth value will be 1 which is the far plane so it will be behind eveything
    pos_clip.w = -pos_cam.z;
    
    gl_Position = pos_clip;
}
/*----------------------------------------------------*/ )_SHAD" } )

/*-----------*/ _SOURCE_SHADER("_env_dome.vert", { R"_SHAD(
$include "_glsl_version.h.glsl"
$include "_global_include.h.vert"

ATTRIB(0)	v3		attrib_pos_model;
ATTRIB(1)	v3		attrib_norm_model;

out Vs_Out {
    smooth	v3		vs_interp_pos_cam;
    //smooth	v3		vs_interp_norm_cam;
    //smooth	v3		vs_interp_pos_model;
};

void main () {
    
    v4 pos_cam = g_transforms.model_to_cam * v4(attrib_pos_model, 1);
    v4 pos_clip = g_cam_to_clip * pos_cam;
    
    gl_Position = pos_clip;
    
    vs_interp_pos_cam =		vec3(pos_cam);
    //vs_interp_norm_cam =	g_transforms.normal_model_to_cam * attrib_norm_model;
    //vs_interp_pos_model =	attrib_pos_model;
}
/*----------------------------------------------------*/ )_SHAD" } )

/*-----------*/ _SOURCE_SHADER("_pbr_dev_notex.vert", { R"_SHAD(
$include "_glsl_version.h.glsl"
$include "_global_include.h.vert"

ATTRIB(0)	v3		attrib_pos_model;
ATTRIB(1)	v3		attrib_norm_model;

out Vs_Out {
    smooth	v3		vs_interp_pos_cam;
    smooth	v3		vs_interp_norm_cam;
};

void main () {
    
    v4 pos_cam = g_transforms.model_to_cam * v4(attrib_pos_model, 1);
    v4 pos_clip = g_cam_to_clip * pos_cam;
    
    gl_Position = pos_clip;
    
    vs_interp_pos_cam =				v3(pos_cam);
    vs_interp_norm_cam =		g_transforms.normal_model_to_cam * attrib_norm_model;
}
/*----------------------------------------------------*/ )_SHAD" } )

/*-----------*/ _SOURCE_SHADER("_pbr_dev_tex.vert", { R"_SHAD(
$include "_glsl_version.h.glsl"
$include "_global_include.h.vert"

ATTRIB(0)	v3		attrib_pos_model;
ATTRIB(1)	v3		attrib_norm_model;
ATTRIB(3)	v2		attrib_uv;

out Vs_Out {
    smooth	v3		vs_interp_pos_cam;
    smooth	v3		vs_interp_norm_cam;
    smooth	v2		vs_interp_uv;
};

void main () {
    
    v4 pos_cam = g_transforms.model_to_cam * v4(attrib_pos_model, 1);
    
    v4 pos_clip = g_cam_to_clip * pos_cam;
    gl_Position = pos_clip;
    
    vs_interp_pos_cam =			pos_cam.xyz;
    vs_interp_norm_cam =	g_transforms.normal_model_to_cam * attrib_norm_model;
    vs_interp_uv =				attrib_uv;
}
/*----------------------------------------------------*/ )_SHAD" } )

/*-----------*/ _SOURCE_SHADER("_pbr_dev_normal_mapped.vert", { R"_SHAD(
$include "_glsl_version.h.glsl"
$include "_global_include.h.vert"

ATTRIB(0)	v3		attrib_pos_model;
ATTRIB(1)	v3		attrib_norm_model;
ATTRIB(2)	v4		attrib_tang_model;
ATTRIB(3)	v2		attrib_uv;

out Vs_Out {
    smooth	v3		vs_interp_pos_cam;
    smooth	v3		vs_interp_norm_cam;
    smooth	v4		vs_interp_tang_cam;
    smooth	v2		vs_interp_uv;
};

void main () {
    
    v4 pos_cam = g_transforms.model_to_cam * v4(attrib_pos_model, 1);
    
    v4 pos_clip = g_cam_to_clip * pos_cam;
    gl_Position = pos_clip;
    
    vs_interp_pos_cam =				pos_cam.xyz;
    vs_interp_norm_cam =		g_transforms.normal_model_to_cam * attrib_norm_model;
    vs_interp_tang_cam =	v4(	g_transforms.normal_model_to_cam * attrib_tang_model.xyz, attrib_tang_model.w );
    vs_interp_uv =					attrib_uv;
    
}
/*----------------------------------------------------*/ )_SHAD" } )

/*-----------*/ _SOURCE_SHADER("_fullscreen_quad.vert", { R"_SHAD(
$include "_glsl_version.h.glsl"
$include "_global_include.h.vert"

ATTRIB(0)	v2		attrib_pos_ndc;
ATTRIB(1)	v2		attrib_uv;

out Vs_Out {
    smooth	v2		vs_interp_uv;
};

void main () {
    gl_Position =				v4(attrib_pos_ndc, 0, 1);
    vs_interp_uv =				attrib_uv;
}
/*----------------------------------------------------*/ )_SHAD" } )

/*-----------*/ _SOURCE_SHADER("_camp_rgba.vert", { R"_SHAD(
$include "_glsl_version.h.glsl"
$include "_global_include.h.vert"

ATTRIB(0)	v4		attrib_pos_clip;
ATTRIB(1)	v4		attrib_col;
ATTRIB(2)	flt		attrib_line_pos;

out Vs_Out {
    noperspective	v4		vs_interp_col;
    smooth			flt		vs_interp_line_pos;
};

void main () {
    gl_Position =				attrib_pos_clip;
    vs_interp_col =				attrib_col;
    vs_interp_line_pos =		attrib_line_pos;
    
}
/*----------------------------------------------------*/ )_SHAD" } )

/*-----------*/ _SOURCE_SHADER("_render_cubemap.vert", { R"_SHAD(
$include "_glsl_version.h.glsl"
$include "_global_include.h.vert"

ATTRIB(0)	v2		attrib_pos_ndc;
ATTRIB(1)	v3		attrib_render_cubemap_dir;

out Vs_Out {
    smooth	v3		vs_interp_render_cubemap_dir;
};

void main () {
    
    gl_Position =					v4(attrib_pos_ndc, 0, 1);
    vs_interp_render_cubemap_dir =	attrib_render_cubemap_dir;
}
/*----------------------------------------------------*/ )_SHAD" } )

//////
// Fragment shaders
//////
/*-----------*/ _SOURCE_SHADER("_shadow_map_directional.frag", { R"_SHAD(
$include "_glsl_version.h.glsl"
$include "_global_include.h.frag"

void main () {
    // Only write depth
}
/*----------------------------------------------------*/ )_SHAD" } )

/*-----------*/ _SOURCE_SHADER("_shadow_map_omnidir.frag", { R"_SHAD(
$include "_glsl_version.h.glsl"
$include "_global_include.h.frag"

in Vs_Out {
    smooth	v3		vs_interp_pos_light;
};

uniform		flt		unif_inv_far;

void main () {
    gl_FragDepth = length(vs_interp_pos_light) * unif_inv_far;
}
/*----------------------------------------------------*/ )_SHAD" } )

/*-----------*/ _SOURCE_SHADER("_solid_color.frag", { R"_SHAD(
$include "_glsl_version.h.glsl"
$include "_global_include.h.frag"

void main () {
    v3 col = g_mat.albedo;
    
    FRAG_DONE(v4(col, 1.0));
}
/*----------------------------------------------------*/ )_SHAD" } )

/*-----------*/ _SOURCE_SHADER("rgba.frag", { R"_SHAD(
$include "_glsl_version.h.glsl"
$include "_global_include.h.frag"

in Vs_Out {
    noperspective	v4		vs_interp_col;
    smooth			flt		vs_interp_line_pos;
};

uniform	sampler2D		main_pass_depth_tex;

void main () {
    
    #if 1
    v2	uv;
    flt	depth;
    uv2	pix_coord;
    {
        v4	coord = gl_FragCoord;
        uv = coord.xy / v2(g_target_res);
        depth = coord.z;
        pix_coord = uv2(coord.xy);
    }
    flt	main_depth = texture(main_pass_depth_tex, uv).r;
    
    if (main_depth < depth && mod(vs_interp_line_pos, 0.125) < 0.125*0.666) {
        discard;
    }
    
    FRAG_DONE(v4(vs_interp_col.rgb, vs_interp_col.a));
    #endif
    
    FRAG_DONE(v4(mix(v3(0,1,0), v3(1), vs_interp_col.a), 1.0));
}
/*----------------------------------------------------*/ )_SHAD" } )

/*-----------*/ _SOURCE_SHADER("_fullscreen_tex.frag", { R"_SHAD(
$include "_glsl_version.h.glsl"
$include "_global_include.h.frag"

in Vs_Out {
    smooth				v2		vs_interp_uv;
};

        uniform	sampler2D		tex;

void main () {
    FRAG_DONE(texture(tex, vs_interp_uv));
}

/*----------------------------------------------------*/ )_SHAD" } )

/*-----------*/ _SOURCE_SHADER("_render_ibl_illuminance.frag", { R"_SHAD(
$include "_glsl_version.h.glsl"

#define PBR_PRERENDER 1
uniform	samplerCube		env_luminance_tex;
$include "pbr.h.frag"

in Vs_Out {
    smooth				v3		vs_interp_render_cubemap_dir;
};

void main () {
    
    v3 illuminance = convolve_illuminance(vs_interp_render_cubemap_dir);
    
    FRAG_DONE(v4(illuminance, 1))
}
/*----------------------------------------------------*/ )_SHAD" } )

/*-----------*/ _SOURCE_SHADER("_render_ibl_luminance_prefilter.frag", { R"_SHAD(
$include "_glsl_version.h.glsl"

#define PBR_PRERENDER 1
uniform	samplerCube		env_luminance_tex;
$include "pbr.h.frag"

in Vs_Out {
    smooth				v3		vs_interp_render_cubemap_dir;
};

void main () {
    
    v3 luminance = prefilter_luminance(vs_interp_render_cubemap_dir);
    
    FRAG_DONE(v4(luminance, 1))
}
/*----------------------------------------------------*/ )_SHAD" } )

/*-----------*/ _SOURCE_SHADER("_render_ibl_brdf_lut.frag", { R"_SHAD(
$include "_glsl_version.h.glsl"

#define PBR_PRERENDER 2
$include "pbr.h.frag"

in Vs_Out {
    smooth	vec2		vs_interp_uv;
};

void main () {
    
    v2 val = integrate_brdf(vs_interp_uv.x, vs_interp_uv.y);
    
    FRAG_DONE(v4(val, 0, 1))
}
/*----------------------------------------------------*/ )_SHAD" } )

/*-----------*/ _SOURCE_SHADER("_render_equirectangular_to_cubemap.frag", { R"_SHAD(
$include "_glsl_version.h.glsl"
$include "_global_include.h.frag"

uniform	sampler2D		env_luminance_tex;

in Vs_Out {
    smooth				v3		vs_interp_render_cubemap_dir;
};

void main () {
    
    v3 dir = normalize(vs_interp_render_cubemap_dir);
    
    v2 uv = v2(atan(-dir.y, dir.x), asin(dir.z));
    uv *= v2(0.1591, 0.3183); // inv_atan
    uv += v2(0.5);
    
    v3 luminance = texture(env_luminance_tex, uv).rgb;
    
    FRAG_DONE(v4(luminance, 1))
}
/*----------------------------------------------------*/ )_SHAD" } )

/*-----------*/ _SOURCE_SHADER("_pbr_dev_notex.frag", { R"_SHAD(
$include "_glsl_version.h.glsl"
$include "pbr.h.frag"

in Vs_Out {
    smooth				v3	vs_interp_pos_cam;
    smooth				v3	vs_interp_norm_cam;
};

void main () {
    
    v4 col = brfd_test(vs_interp_pos_cam, normalize(vs_interp_norm_cam),
            g_mat.albedo, g_mat.metallic, g_mat.roughness, g_mat.IOR);
    
    FRAG_DONE(col)
}
/*----------------------------------------------------*/ )_SHAD" } )

/*-----------*/ _SOURCE_SHADER("_pbr_dev_albedo.frag", { R"_SHAD(
$include "_glsl_version.h.glsl"
$include "pbr.h.frag"

in Vs_Out {
    smooth	v3		vs_interp_pos_cam;
    smooth	v3		vs_interp_norm_cam;
    smooth	v2		vs_interp_uv;
};

uniform	sampler2D			albedo_tex;

void main () {
    
    v3	albedo_sample = texture(albedo_tex, vs_interp_uv).rgb;
    v3	albedo =		g_mat.albedo * albedo_sample;
    
    v4 col = brfd_test(vs_interp_pos_cam, normalize(vs_interp_norm_cam),
            albedo, g_mat.metallic, g_mat.roughness, g_mat.IOR);
    
    FRAG_DONE(col)
}
/*----------------------------------------------------*/ )_SHAD" } )

/*-----------*/ _SOURCE_SHADER("_pbr_dev_normal_mapped.frag", { R"_SHAD(
$include "_glsl_version.h.glsl"
$include "pbr.h.frag"

in Vs_Out {
    smooth	v3	vs_interp_pos_cam;
    smooth	v3	vs_interp_norm_cam;
    smooth	v4	vs_interp_tang_cam;
    smooth	v2	vs_interp_uv;
};

uniform	sampler2D			albedo_tex;
uniform	sampler2D			normal_tex;

void main () {
    
    v3	albedo_sample = texture(albedo_tex, vs_interp_uv).rgb;
    v3	albedo =		g_mat.albedo * albedo_sample;
    
    v3	normal_dir_cam = normal_mapping(vs_interp_pos_cam, vs_interp_tang_cam, vs_interp_norm_cam, vs_interp_uv, normal_tex);
    
    v4 col = brfd_test(vs_interp_pos_cam, normal_dir_cam,
            albedo, g_mat.metallic, g_mat.roughness, g_mat.IOR);
    
    FRAG_DONE(col)
    
}
/*----------------------------------------------------*/ )_SHAD" } )

/*-----------*/ _SOURCE_SHADER("_pbr_dev_cerberus.frag", { R"_SHAD(
$include "_glsl_version.h.glsl"
$include "pbr.h.frag"

in Vs_Out {
    smooth	v3	vs_interp_pos_cam;
    smooth	v3	vs_interp_norm_cam;
    smooth	v4	vs_interp_tang_cam;
    smooth	v2	vs_interp_uv;
};

uniform		sampler2D			albedo_tex;
uniform		sampler2D			normal_tex;
uniform		sampler2D			metallic_tex;
uniform		sampler2D			roughness_tex;

void main () {
    
    v3 normal_dir_cam = normal_mapping(vs_interp_pos_cam, vs_interp_tang_cam, vs_interp_norm_cam, vs_interp_uv, normal_tex);
    
    v3	albedo =	g_mat.albedo *		texture(albedo_tex, vs_interp_uv).rgb;
    flt	metallic =	g_mat.metallic *	texture(metallic_tex, vs_interp_uv).r;
    flt	roughness =	g_mat.roughness *	texture(roughness_tex, vs_interp_uv).r;
    
    v4 col = brfd_test(vs_interp_pos_cam, normal_dir_cam,
            albedo, metallic, roughness, g_mat.IOR);
    
    FRAG_DONE(col)
    
}
/*----------------------------------------------------*/ )_SHAD" } )

/*-----------*/ _SOURCE_SHADER("_pbr_dev_ugly.frag", { R"_SHAD(
$include "_glsl_version.h.glsl"
$include "pbr.h.frag"

in Vs_Out {
    smooth	v3	vs_interp_pos_cam;
    smooth	v3	vs_interp_norm_cam;
    smooth	v2	vs_interp_uv;
};

uniform	sampler2D			roughness_tex;

void main () {
    
    flt	roughness =	g_mat.roughness *	texture(roughness_tex, vs_interp_uv).r;
    roughness = pow(roughness, 1.0/2.2);
    
    //DBG_COL(roughness);
    
    v4 col = brfd_test(vs_interp_pos_cam, normalize(vs_interp_norm_cam),
            g_mat.albedo, g_mat.metallic, roughness, g_mat.IOR);
    
    FRAG_DONE(col)
    
}
/*----------------------------------------------------*/ )_SHAD" } )

/*-----------*/ _SOURCE_SHADER("_pbr_dev_nanosuit.frag", { R"_SHAD(
$include "_glsl_version.h.glsl"
$include "pbr.h.frag"

in Vs_Out {
    smooth	v3	vs_interp_pos_cam;
    smooth	v3	vs_interp_norm_cam;
    smooth	v4	vs_interp_tang_cam;
    smooth	v2	vs_interp_uv;
};

uniform	sampler2D			diffusse_emmissive_tex;
uniform	sampler2D			normal_tex;
uniform	sampler2D			specular_roughness_tex;

void main () {
    
    v3 normal_dir_cam = normal_mapping(vs_interp_pos_cam, vs_interp_tang_cam, vs_interp_norm_cam, vs_interp_uv, normal_tex);
    
    v4	diff_emmis_sample = texture(diffusse_emmissive_tex, vs_interp_uv).rgba;
    v4	spec_rough_sample = texture(specular_roughness_tex, vs_interp_uv).rgba;
    
    v3	diffuse_col = diff_emmis_sample.xyz;
    v3	specular_col = spec_rough_sample.xyz;
    
    v3	albedo =	diffuse_col;
    flt	metallic =	avg_xyz(specular_col);
    flt	roughness =	spec_rough_sample.a;
    
    albedo = mix(albedo, specular_col, metallic);
    
    v4 col = brfd_test(vs_interp_pos_cam, normal_dir_cam,
            albedo, metallic, roughness, g_mat.IOR);
    
    FRAG_DONE(col)
}
/*----------------------------------------------------*/ )_SHAD" } )
