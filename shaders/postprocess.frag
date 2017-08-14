$include "_glsl_version.h.glsl"
$include "_global_include.h.frag"

#define QUART_UR(ratio) \
		v2 coord = gl_FragCoord.xy / g_target_res; \
		v2 uv = v2(	1.0 -(1.0 -coord.x)/ratio, \
					1.0 -(1.0 -coord.y)/ratio ); \
		if ( coord.x > (1.0 -ratio) && coord.y > (1.0 -ratio))
#define QUART_UL(ratio) \
		v2 coord = gl_FragCoord.xy / g_target_res; \
		v2 uv = v2(	coord.x/ratio, \
					1.0 -(1.0 -coord.y)/ratio ); \
		if ( coord.x < ratio && coord.y > (1.0 -ratio))
#define QUART_LL(ratio) \
		v2 coord = gl_FragCoord.xy / g_target_res; \
		v2 uv = v2(	coord.x/ratio, \
					coord.y/ratio ); \
		if ( coord.x < ratio && coord.y < ratio)
#define QUART_LR(ratio) \
		v2 coord = gl_FragCoord.xy / g_target_res; \
		v2 uv = v2(	1.0 -(1.0 -coord.x)/ratio, \
					coord.y/ratio ); \
		if ( coord.x > (1.0 -ratio) && coord.y < ratio)

flt mymod (flt x, flt y) {
	flt ret = mod(x, y);
	if (ret < 0.0) ret += y;
	return ret;
}

in Vs_Out {
	smooth	v2			vs_interp_uv;
};

uniform	sampler2D		main_pass_lumi_tex;
uniform	sampler2D		main_pass_bloom_0_tex;
uniform	sampler2D		main_pass_bloom_1_tex;
uniform	sampler2D		main_pass_bloom_2_tex;
uniform	sampler2D		main_pass_bloom_3_tex;
uniform	sampler2D		main_pass_bloom_4_tex;
uniform	sampler2D		dbg_line_pass;
uniform	sampler2D		dbg_tex_0;
uniform	samplerCube		dbg_tex_1;

uniform	flt				bloom_intensity;

const v2 kernel_offs[3*3] = v2[] (
	v2(-1,-1), v2( 0,-1), v2(+1,-1),
	v2(-1, 0), v2( 0, 0), v2(+1, 0),
	v2(-1,+1), v2( 0,+1), v2(+1,+1) );

v2 polar_coords (in v2 uv) {
	return v2( length(uv), atan(-uv.x, -uv.y) ); // -180 - +180 deg
}
v2 polar_coords_nonneg (in v2 uv) {
	return v2( length(uv), atan(-uv.x, -uv.y) +PI ); // 0-360 deg
}
v2 polar_coords_01 (in v2 uv) {
	return v2( length(uv), atan(-uv.x, -uv.y)/(2*PI) +0.5 ); // 0-360 deg as 0-1
}

v3 hsl_to_rgb (in v3 hsl) {
	flt hue = hsl.x;
	flt sat = hsl.y;
	flt lht = hsl.z;
	
	flt hue6 = hue*6.0;
	
	flt c = sat*(1.0 -abs(2.0*lht -1.0));
	flt x = c * (1.0 -abs(mod(hue6, 2.0) -1.0));
	flt m = lht -(c/2.0);
	
	v3 rgb;
	if (		hue6 < 1.0 )	rgb = v3(c,x,0);
	else if (	hue6 < 2.0 )	rgb = v3(x,c,0);
	else if (	hue6 < 3.0 )	rgb = v3(0,c,x);
	else if (	hue6 < 4.0 )	rgb = v3(0,x,c);
	else if (	hue6 < 5.0 )	rgb = v3(x,0,c);
	else						rgb = v3(c,0,x);
	rgb += v3(m);
	
	return pow(rgb, v3(2.2)); // to linear
}

v3 rgb_to_hsl (in v3 rgb) {
	rgb = pow(rgb, v3(1.0/2.2));
	
	flt cmax = max( max(rgb.r, rgb.g), rgb.b );
	flt cmin = min( min(rgb.r, rgb.g), rgb.b );
	flt delt = cmax -cmin;
	
	flt lht = mix(cmax, cmin, 0.5);
	flt sat = delt == 0.0 ? 0.0 : delt/(1.0 -abs(2.0*lht -1.0));
	flt hue;
	if (delt == 0.0)							hue = 0.0;
	else if (rgb.r > rgb.g && rgb.r > rgb.b)	hue = mod( (rgb.g -rgb.b)/delt, 6.0 ) / 6.0;
	else if (rgb.g > rgb.r && rgb.g > rgb.b)	hue = ( (rgb.b -rgb.r)/delt +2.0 ) / 6.0;
	else 										hue = ( (rgb.r -rgb.g)/delt +4.0 ) / 6.0;
	
	return v3(hue, sat, lht);
}

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
	v3 bloom_0 =	texture(main_pass_bloom_0_tex, vs_interp_uv).rgb;
	v3 bloom_1 =	texture(main_pass_bloom_1_tex, vs_interp_uv).rgb;
	v3 bloom_2 =	texture(main_pass_bloom_2_tex, vs_interp_uv).rgb;
	v3 bloom_3 =	texture(main_pass_bloom_3_tex, vs_interp_uv).rgb;
	v3 bloom_4 =	texture(main_pass_bloom_4_tex, vs_interp_uv).rgb;
	
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
	
	v4 dbg_line = texture(dbg_line_pass, vs_interp_uv);
	
	col += (bloom_0 +bloom_1 +bloom_2 +bloom_3 +bloom_4) * bloom_intensity;
	
	if (0) {
		col = v3(1.0 -dbg_line.a)*col +v3(dbg_line.a)*dbg_line.rgb;
	}
	
	{
		#if 0
		{
			QUART_UR(0.3) {
				v3	val = texture(main_pass_bloom_0_tex, uv).rgb;
					val += texture(main_pass_bloom_1_tex, uv).rgb;
					val += texture(main_pass_bloom_2_tex, uv).rgb;
					val += texture(main_pass_bloom_3_tex, uv).rgb;
					val += texture(main_pass_bloom_4_tex, uv).rgb;
				DBG_COL(map(val, 0.0, 1.0));
			}
		}
		#endif
		
		#if 0
		{
			QUART_UR(0.3) {
				v2 polar = polar_coords_01(uv*v2(2) -v2(1));
				
				col = hsl_to_rgb(v3(polar.y, polar.x, 0.75));
			}
		}
		{
			QUART_UL(0.3) {
				v2 polar = polar_coords_01(uv*v2(2) -v2(1));
				
				col = hsl_to_rgb(v3(18.0 / 360.0, uv.y, uv.x));
			}
		}
		#endif
		
		#if 0
		QUART_UR(0.3) {
			DBG_COL(textureLod(dbg_tex_0, uv, 6.0).rgb);
		}
		
		//QUART_UR(0.3) {
		//	DBG_COL(map(texture(dbg_tex_1, uv_to_cubemap_dir(uv)).r, 0.0, 1.0));
		//}
		#endif
	}
	
	if (1) { // keeps colors even if one component clips
		flt a = 0.95;
		flt b = 1.0/6.0;
		
		flt max_col = max( max(col.r, col.g), col.b );
		if (max_col > a) {
			flt t = min(b * log(max_col +(1.0 -a)), 1.0);
			
			v3 hsl = rgb_to_hsl(col);
			
			hsl.y = mix(hsl.y, 1.0, t);
			
			col = hsl_to_rgb(hsl);
			
		}
	}
	
	FRAG_DONE(v4(col, 1))
	
}
