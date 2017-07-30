$include "_glsl_version.h.glsl"
$include "_global_include.h.frag"

in Vs_Out {
	smooth	v2			vs_interp_uv;
};

uniform	sampler2D		main_pass_lumi_tex;
uniform	sampler2D		main_pass_bloom_0_tex;
uniform	sampler2D		main_pass_bloom_1_tex;
uniform	sampler2D		main_pass_bloom_2_tex;
uniform	sampler2D		dbg_line_pass;
uniform	sampler2D		dbg_tex_0;
uniform	samplerCube		dbg_tex_1;

const v2 kernel_offs[3*3] = v2[] (
	v2(-1,-1), v2( 0,-1), v2(+1,-1),
	v2(-1, 0), v2( 0, 0), v2(+1, 0),
	v2(-1,+1), v2( 0,+1), v2(+1,+1) );

flt luminance (v3 col) {
	return dot(col, v3(0.2126, 0.7152, 0.0722));
}

v3 uv_to_cubemap_dir (v2 uv) { // u 0/0.25/0.5/0.75 -> -y/-x/+y/+x   v 0/1 -> -z/+z
	flt u = uv.x * 2*PI;
	flt v = uv.y * PI;
	
	v2	xy = v2(-sin(u),-cos(u));
	
	return v3(v2(sin(v))*xy, -cos(v));
}

void main () {
	
	v3	luminance =	texture(main_pass_lumi_tex, vs_interp_uv).rgb;
	//v3 bloom_0 =	texture(main_pass_bloom_0_tex, vs_interp_uv).rgb;
	//v3 bloom_1 =	texture(main_pass_bloom_1_tex, vs_interp_uv).rgb;
	//v3 bloom_2 =	texture(main_pass_bloom_2_tex, vs_interp_uv).rgb;
	
	#if 0
	v3	col;
	
	//DBG_RIGHT {
		v3	a = v3(0.18);
		//v3	a = v3(get_mouse_pos().x);
		
		v3	lumi_white = v3(6.0);
		//v3	lumi_white = v3(mix(1.0, 20.0, get_mouse_pos().x));
		
		v3	avg_luminance = v3(exp(g_avg_log_luminance));
		
		col = (a / avg_luminance) * luminance;
		lumi_white = (a / avg_luminance) * lumi_white;
		
		col = (col * (1.0 +(col / (lumi_white*lumi_white)))) / (col +1.0);
	//} else {
	//	col = luminance / (luminance +1.0);
	//}
	#else
	v3 col = luminance;
	#endif
	
	#if 1
	{
		flt ratio = 0.3;
		v2 coord = gl_FragCoord.xy / g_target_res;
		if ( all(not(lessThan(coord, v2(1.0 -ratio)))) ) {
			v2 uv = (coord -v2(1.0 -ratio)) / v2(ratio);
			DBG_COL(map(texture(dbg_tex_0, uv).r, 0.0, 1.0));
		}
	}
	#elif 0
	{
		flt ratio = 0.3;
		v2 coord = gl_FragCoord.xy / g_target_res;
		if ( all(not(lessThan(coord, v2(1.0 -ratio)))) ) {
			v2 uv = (coord -v2(1.0 -ratio)) / v2(ratio);
			DBG_COL(map(texture(dbg_tex_1, uv_to_cubemap_dir(uv)).r, 0.0, 1.0));
		}
	}
	#endif
	
	v4 dbg_line = texture(dbg_line_pass, vs_interp_uv);
	col = v3(1.0 -dbg_line.a)*col +v3(dbg_line.a)*dbg_line.rgb;
	
	FRAG_DONE(v4(col, 1))
	
}
