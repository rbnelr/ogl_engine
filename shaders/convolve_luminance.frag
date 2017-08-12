$include "_glsl_version.h.glsl"
$include "_global_include.h.frag"

in Vs_Out {
	smooth	v2		vs_interp_uv;
};

uniform	sampler2D	tex;

uniform	ui			mip_pass;
uniform	v2			blur_dist;

flt luminance (v3 col) {
	return dot(col, v3(0.2126, 0.7152, 0.0722));
}

const v2	GAUSS_OFFS[3*3] = v2[](
	v2(-1,+1),	v2( 0,+1),	v2(+1,+1),
	v2(-1, 0),	v2( 0, 0),	v2(+1, 0),
	v2(-1,-1),	v2( 0,-1),	v2(+1,-1)
);
const flt	GAUSS_KERNEL[3*3] = flt[](
	1.0/16.0,	1.0/ 8.0,	1.0/16.0,
	1.0/ 8.0,	1.0/ 4.0,	1.0/ 8.0,
	1.0/16.0,	1.0/ 8.0,	1.0/16.0
);

#define NO_BLUR 1

void main () {
	
	v2 blur_dist_ = blur_dist * 0.5;
	
	v3	avg_lumi =		v3(0);
	flt	avg_log_lumi =	0.0;
	
	if (mip_pass == ui(0)) {
		#if !NO_BLUR
		for (ui i=ui(0); i<ui(9); ++i) {
			v3	lumi = texture(tex, vs_interp_uv +(GAUSS_OFFS[i] * blur_dist_)).rgb;
			
			avg_lumi +=		v3(GAUSS_KERNEL[i]) * lumi;
			avg_log_lumi += GAUSS_KERNEL[i] * log(luminance(lumi) +0.00001);
		}
		
		#else // Dont do blur
		v3	lumi = texture(tex, vs_interp_uv).rgb;
		avg_lumi =		lumi;
		avg_log_lumi =	log(luminance(lumi) +0.00001);
		#endif
		
	} else {
		#if !NO_BLUR
		for (ui i=ui(0); i<ui(9); ++i) {
			v4	sampl = v4(GAUSS_KERNEL[i]) * texture(tex, vs_interp_uv +(GAUSS_OFFS[i] * blur_dist_));
			
			avg_lumi +=		sampl.rgb;
			avg_log_lumi += sampl.a;
		}
		
		#else // Dont do blur
		v4	sampl = texture(tex, vs_interp_uv);
		avg_lumi =		sampl.rgb;
		avg_log_lumi =	sampl.a;
		#endif
		
	}
	
	FRAG_DONE(v4(avg_lumi, avg_log_lumi));
}
