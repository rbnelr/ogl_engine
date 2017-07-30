$include "_glsl_version.h.glsl"
$include "_global_include.h.frag"

in Vs_Out {
	smooth	v2		vs_interp_uv;
};

uniform	sampler2D	main_pass_lumi_tex;
uniform	sampler1D	gaussian_LUT_tex;

uniform	flt			gauss_res;

uniform	ui			gauss_axis;
uniform	flt			gauss_kernel_size;
uniform	flt			gauss_coeff;

flt luminance (v3 col) {
	return dot(col, v3(0.2126, 0.7152, 0.0722));
}

flt my_curve (flt x) {
	flt	a = 0.95;
	flt	b = 1.3;
	flt	s = 0.5;
	
	if (x <= a) {
		return 0.0;
	} else if (x <= b) {
		flt tmp = x -a;
		return (s*(tmp*tmp)) / (2.0 * (b -a));
	} else {
		flt tmp = b -a;
		flt	fb = (s*(tmp*tmp)) / (2.0 * (b -a));
		return s*x -s*b +fb;
	}
}

v3 curve (v3 luminance) {
	
	v3	col = luminance * v3(0.2);
	
	flt	intens = dot(col, v3(0.3));
	
	flt	bloom = my_curve(intens);
	
	//if (abs(get_screen_pos().y -my_curve(get_screen_pos().x * 2.0)) < 0.005) {
	//	DBG_COL(v3(0,1,0));
	//}
	
	return col * v3(bloom / (intens +0.00001));
}

void main () {
	
	v2	axis = v2(0);
		axis[gauss_axis] = 1.0;
	
	flt	gauss_kernel_size_ = gauss_kernel_size;
	//if (in_rect(get_mouse_pos(), v2(0), v2(1))) {
	//	gauss_kernel_size_ *= pow(2.0, mix(-1.0, +1.0, get_mouse_pos().x));
	//}
	flt	kernel_dist = gauss_kernel_size_ * 0.5;
	
	ui	half_samples = ui(max(gauss_res * kernel_dist, 0.0) +1.0); // round up always so that we never get 0
	
	kernel_dist = flt(half_samples) / gauss_res;
	
	v3	col = texture(main_pass_lumi_tex, vs_interp_uv).rgb;
	
	if (gauss_axis == ui(0)) { // combined threshold and gaussian blur pass
		col = curve(col);
	}
	
	col = max( col*v3(texture(gaussian_LUT_tex, 0.0).r), v3(0) );
	
	for (ui i=ui(1); i<half_samples; ++i) {
		flt	u = flt(i)/flt(half_samples -ui(1));
		v2	uv = v2(u * kernel_dist) * axis;
		
		v3	l = texture(main_pass_lumi_tex, vs_interp_uv +uv).rgb;
		v3	r = texture(main_pass_lumi_tex, vs_interp_uv -uv).rgb;
		
		if (gauss_axis == ui(0)) {
			l = curve(l);
			r = curve(r);
		}
		
		flt	gauss = texture(gaussian_LUT_tex, u).r;
		
		col += max( (l +r)*v3(gauss), v3(0) );
	}
	
	col /= half_samples;
	col *= gauss_coeff;
	
	FRAG_DONE(v4(col, 1))
}
