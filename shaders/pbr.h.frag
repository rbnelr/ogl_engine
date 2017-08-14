$include "_global_include.h.frag"

const flt IOR_air =		1.00;
const flt IOR_water = 	1.33;
const flt IOR_ice =		1.309;
const flt IOR_glass =	1.52;
const flt IOR_diamond =	2.42;

v3 fresnel_schlick (flt IORs, flt IORd, v3 albedo, flt metallic, flt dvh) {
	
	flt	diel_F0 = (IORs -IORd) / (IORs +IORd);
	
	diel_F0 = diel_F0*diel_F0;
	
	v3	F0 = mix(v3(diel_F0), albedo, metallic);
	
	return F0 +(1.0 -F0)*pow(1.0 -dvh, 5.0);
}
v3 fresnel_schlick_roughness (flt IORs, flt IORd, v3 albedo, flt metallic, flt dvh, flt roughness) {
	
	flt	diel_F0 = (IORs -IORd) / (IORs +IORd);
	
	diel_F0 = diel_F0*diel_F0;
	
	v3	F0 = mix(v3(diel_F0), albedo, metallic);
	
	return F0 +( max(v3(1.0 -roughness), F0) -F0 )*pow(1.0 -dvh, 5.0);
}

flt distribution_GGX (flt roughness, flt dhn) {
	flt	a = roughness*roughness;
	flt	a2 = a*a;
	
	flt	tmp = (dhn*dhn)*(a2 -1.0) +1.0;
	flt denom = PI * (tmp*tmp);
	
	return a2 / (denom +0.000001);
}

flt geometry_schlick_GGX_direct (flt roughness, flt dvn) {
	flt	r = roughness +1.0;
	flt	k = (r*r) / 8.0;
	
	flt	denom = dvn*(1.0 -k) +k;
	
	return dvn / (denom +0.000001);
}
flt geometry_smith_direct (flt roughness, flt dvn, flt dln) {
	return geometry_schlick_GGX_direct(roughness, dvn) * geometry_schlick_GGX_direct(roughness, dln);
}

flt geometry_schlick_GGX_IBL (flt roughness, flt dvn) {
	flt	r = roughness;
	flt	k = (r*r) / 2.0;
	
	flt	denom = dvn*(1.0 -k) +k;
	
	return dvn / denom;
}
flt geometry_smith_IBL (flt roughness, flt dvn, flt dln) {
	return geometry_schlick_GGX_IBL(roughness, dvn) * geometry_schlick_GGX_IBL(roughness, dln);
}

#ifndef PBR_PRERENDER
uniform	samplerCube			env_luminance_tex;
uniform	samplerCube			env_illuminance_tex;
uniform	sampler2D			brdf_LUT_tex;

// MAX_LIGHTS_COUNT = 8
uniform	sampler2DShadow		shadow_2d_0_tex;
uniform	sampler2DShadow		shadow_2d_1_tex;
uniform	sampler2DShadow		shadow_2d_2_tex;
uniform	sampler2DShadow		shadow_2d_3_tex;
uniform	sampler2DShadow		shadow_2d_4_tex;
uniform	sampler2DShadow		shadow_2d_5_tex;
uniform	sampler2DShadow		shadow_2d_6_tex;
uniform	sampler2DShadow		shadow_2d_7_tex;

uniform	samplerCubeShadow	shadow_cube_0_tex;
uniform	samplerCubeShadow	shadow_cube_1_tex;
uniform	samplerCubeShadow	shadow_cube_2_tex;
uniform	samplerCubeShadow	shadow_cube_3_tex;
uniform	samplerCubeShadow	shadow_cube_4_tex;
uniform	samplerCubeShadow	shadow_cube_5_tex;
uniform	samplerCubeShadow	shadow_cube_6_tex;
uniform	samplerCubeShadow	shadow_cube_7_tex;

flt sample_shadow_2d_tex (ui light_indx, v3 coords) {
	switch (light_indx) {
		case ui(0):		return texture(shadow_2d_0_tex, coords); // .r this is an error on notebook nv gpu
		case ui(1):		return texture(shadow_2d_1_tex, coords);
		case ui(2):		return texture(shadow_2d_2_tex, coords);
		case ui(3):		return texture(shadow_2d_3_tex, coords);
		case ui(4):		return texture(shadow_2d_4_tex, coords);
		case ui(5):		return texture(shadow_2d_5_tex, coords);
		case ui(6):		return texture(shadow_2d_6_tex, coords);
		case ui(7):		return texture(shadow_2d_7_tex, coords);
		
		default:	return 1.0;
	}
}
flt sample_shadow_cube_tex (ui light_indx, v4 coords) {
	switch (light_indx) {
		case ui(0):		return texture(shadow_cube_0_tex, coords);
		case ui(1):		return texture(shadow_cube_1_tex, coords);
		case ui(2):		return texture(shadow_cube_2_tex, coords);
		case ui(3):		return texture(shadow_cube_3_tex, coords);
		case ui(4):		return texture(shadow_cube_4_tex, coords);
		case ui(5):		return texture(shadow_cube_5_tex, coords);
		case ui(6):		return texture(shadow_cube_6_tex, coords);
		case ui(7):		return texture(shadow_cube_7_tex, coords);
		
		default:	return 1.0;
	}
}

v3 normal_mapping (v3 interp_pos_cam, v4 interp_tang_cam, v3 interp_norm_cam, v2 uv, sampler2D normal_tex) {
	
	v3 norm = normalize(interp_norm_cam);
	
	m3 TBN_tang_to_cam;
	{
		v3	t =			interp_tang_cam.xyz;
		flt	b_sign =	interp_tang_cam.w;
		v3	b;
		v3	n =			norm;
		
		t = normalize( t -(dot(t, n) * n) );
		b = cross(n, t);
		b *= b_sign;
		
		TBN_tang_to_cam = m3(t, b, n);
	}
	
	v3 normal_tang_sample = texture(normal_tex, uv).rgb;
	v3 norm_dir_tang = normalize((normal_tang_sample * v3(2.0)) -v3(1.0));
	v3 norm_dir_cam = TBN_tang_to_cam * norm_dir_tang;
	
	v3 view = normalize(-interp_pos_cam);
	
	const flt ARTIFACT_FIX_THRES = 0.128;
	if (dot(view, norm_dir_cam) < ARTIFACT_FIX_THRES) {
		norm_dir_cam = mix(norm_dir_cam, norm, map(dot(view, norm_dir_cam), ARTIFACT_FIX_THRES, 0));
		//DBG_COL(map(dot(view, norm_dir_cam), ARTIFACT_FIX_THRES, 0));
	}
	
	//DBG_COL(norm_dir_cam * v3(-1,-1,1));
	//DBG_COL(pow(normal_tang_sample, v3(2.2)));
	
	return norm_dir_cam;
}

v3 get_light (ui i, v3 pos_cam, v3 norm_cam, out v3 luminance) {
	
	Light l = g_lights[i];
	
	if (l.light_vec_cam.w == 0.0) {
		luminance = l.power / v3(550);
		v3 light_cam = l.light_vec_cam.xyz;
		if (l.shad_i != ui(-1)) {
			v3	pos_light_tex = (l.cam_to_light * v4(pos_cam, 1)).xyz * v3(0.5) +v3(0.5);
			if (pos_light_tex.z <= 1.0) {
				
				//flt	min_bias = 1.0 / 2048;
				//flt	max_bias = 1.0 / 1024;
				//
				//flt	bias = max(max_bias * (1.0 -dot(light_cam, norm_cam)), min_bias);
				//
				//pos_light_tex.z -= bias;
				
				flt	depth_sample = sample_shadow_2d_tex(l.shad_i, pos_light_tex);
				luminance *= depth_sample;
			}
			
		}
		
		return light_cam;
	}
	
	v3	light_pos = l.light_vec_cam.xyz;
	v3	diff = light_pos -pos_cam;
	flt	dist = sqrt(dot(diff, diff));
	
	flt	light_radius = 32.0;
	
	v3	light_lumen = l.power;
	v3	light_candela = light_lumen / (4.0 * PI);
	
	//light_radius *= get_mouse_pos().x * get_mouse_pos().x;
	
	//if (g_lightbulb_indx != ui(-1) && i == g_lightbulb_indx) {
	//	light_radius = 0.2; // emmissive
	//	light_power *= 32.0;
	//}
	
	flt	tmp = max(1.0 -pow(dist/light_radius, 4.0), 0.0);
	tmp = tmp*tmp; // Scale to zero at light_radius to make light influence finite
	
	flt	h = 1.41;
	flt	solid_angle = tmp * 0.5 * (1.0 -(dist / sqrt(h*h +dist*dist)));
	
	luminance = light_candela * v3(solid_angle); // half of energy of the light at distance 0
	if (l.shad_i != ui(-1)) {
		v3	light_to_frag = (l.cam_to_light * v4(pos_cam, 1)).xyz;
		v4	coords;
		coords.xyz = light_to_frag;
		coords.w = length(light_to_frag) * g_shadow1_inv_far;
		
		flt	depth_sample = sample_shadow_cube_tex(l.shad_i, coords);
		luminance *= depth_sample;
		
	}
	
	return normalize(diff);
}

v3 brfd_analytical_light (	v3 pos_cam, v3 view, v3 norm, flt dvn,  v3 albedo, flt metallic, flt roughness,
							flt IORs, flt IORd, v3 lightbulb_emissive,  ui light_i ) {
	
	v3	light_luminance;
	v3	light =	get_light(light_i, pos_cam, norm, light_luminance);
	v3	half_ =	normalize(light +view);
	
	if (g_lightbulb_indx != ui(-1) && light_i == g_lightbulb_indx) {
		//DBG_COL(v3(0,1,0));
		return light_luminance * lightbulb_emissive; // emmissive
	}
	
	flt dln =	max(dot(light, norm), 0.0);
	flt dhn =	max(dot(half_, norm), 0.0);
	flt dvh =	max(dot(view, half_), 0.0);
	
	// Fix the case where a half angle gets noisy because light and view vectors are almost 180 deg apart
	//half_ = mix(half_, norm, map(dvh, 0.05, 0.0)); // TODO: see how this affects real render results
	
	v3	F = fresnel_schlick(IORs, IORd, albedo, metallic, dvh);
	flt	D = distribution_GGX(roughness, dhn);
	flt	G = geometry_smith_direct(roughness, dvn, dln);
	
	v3	kS = F;
	v3	kD = v3(1.0) -F;
	
	kD = mix(kD, v3(0), v3(metallic));
	
	v3	brdf = (F*v3(D)*v3(G)) / (4 * dvn * dln +0.000001);
	
	v3	specular = brdf;
	v3	diffuse = kD * (albedo / v3(PI));
	
	v3	luminance = (diffuse +specular) * (light_luminance * v3(dln));
	
	if (dln < 0.0 || dvn < 0.0) {
		luminance = v3(0);
	}
	
	return luminance;
	
}

v3 brfd_IBL (				v3 view, v3 norm, flt dvn,  v3 albedo, flt metallic, flt roughness,
							flt	IORs, flt IORd ) {
	
	v3	diffuse;
	{
		v3	F = fresnel_schlick_roughness(IORs, IORd, albedo, metallic, dvn, roughness);
		
		v3	kS = F;
		v3	kD = v3(1.0) -F;
		
		kD = mix(kD, v3(0), v3(metallic));
		
		v3	env_illuminance_sample = texture(env_illuminance_tex, g_camera_to_world * norm).rgb;
		
		diffuse = kD * albedo * env_illuminance_sample;
	}
	
	v3	specular = v3(0);
	#if 1
	{
		v3	refl = reflect(-view, norm);
		
	//	DBG_COL(refl);
		
		v3	env_prefiltered = textureLod(env_luminance_tex, g_camera_to_world * refl,
				roughness * (g_luminance_prefilter_mip_count -1.0)).rgb;
		
	//	DBG_COL(env_prefiltered);
		
		v3	F = fresnel_schlick_roughness(IORs, IORd, albedo, metallic, dvn, roughness);
		v2	env_brdf = texture(brdf_LUT_tex, v2(dvn, roughness)).rg;
		
		specular = env_prefiltered * (F*v3(env_brdf.x) +v3(env_brdf.y));
		
	}
	#endif
	
	v3	luminance = diffuse +specular;
	return luminance;
}

// everything in cam space
v4 brfd_test (	v3 pos_cam, v3 normal_dir, v3 albedo, flt metallic, flt roughness, flt IOR, v3 emissive ) {
	
	//DBG_COL(roughness);
	
	flt IORs = IOR_air;
	flt IORd = IOR;
	
	v3	view =	normalize(-pos_cam);
	v3	norm =	normal_dir;
	
	flt dvn =	max(dot(view, norm), 0.0);
	
	v3 luminance = v3(0);
	
	#if 1
	for (ui i=ui(0); i<g_lights_count; ++i) {
		luminance += brfd_analytical_light(	pos_cam, view, norm, dvn,  albedo, metallic, roughness, IORs, IORd, emissive,  i );
	}
	#endif
	
	#if 0
	luminance += brfd_IBL(	view, norm, dvn,  albedo, metallic, roughness,  IORs, IORd);
	#endif
	
	#if 0
	{
		v2 ru = v2(g_target_res) -v2(gl_FragCoord);
		if ( all(not(greaterThan(ru, v2(512)))) ) {
			v2 uv = v2(1) -(ru / v2(512));
			DBG_COL(texture(brdf_LUT_tex, uv).rgb);
		}
	}
	#endif
	
	return v4(luminance, 1);
	
}
v4 brfd_test (	v3 pos_cam, v3 normal_dir, v3 albedo, flt metallic, flt roughness, flt IOR ) {
	return brfd_test(pos_cam, normal_dir, albedo, metallic, roughness, IOR, v3(0));
}
#else

flt radical_inverse_VdC (ui bits) {
	#if 1
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	#else
	bits = bitfieldReverse(bits);
	#endif
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
v2 hammersley (ui i, ui N) {
	return v2(flt(i)/flt(N), radical_inverse_VdC(i));
}

v3 importance_sample_GGX (v2 Xi, flt roughness, v3 norm) {
	
	flt	a = roughness*roughness;
	
	flt	phi = Xi.x * DEG_360;
	
	flt	cos_theta = sqrt( (1.0 -Xi.y) / (1.0 +(a*a -1)*Xi.y) );
	flt	sin_theta = sqrt(1.0 -cos_theta*cos_theta);
	
	v3	half_ = v3(	sin_theta * cos(phi),
					sin_theta * sin(phi),
					cos_theta );
	
	//v3	up = abs(norm.z) < 0.999 ? v3(0,0,1) : v3(1,0,0);
	v3	up = v3(0,0,1);
	
	v3	tang_x = normalize( cross(up, norm) );
	v3	tang_y = cross(norm, tang_x);
	
	m3	tang_to_cam = m3(tang_x, tang_y, norm);
	
	v3	sample_cam = tang_to_cam * half_;
	
	return sample_cam;
}

#if PBR_PRERENDER == 1
v3 prefilter_luminance (v3 interp_render_cubemap_dir) {
	
	flt	roughness = g_luminance_prefilter_roughness;
	
	v3	norm = normalize(interp_render_cubemap_dir);
	v3	view = norm;
	
	//ui samples_count = ui( pow(2.0, flt(g_luminance_prefilter_mip_indx)) * g_luminance_prefilter_base_samples );
	ui samples_count = g_luminance_prefilter_base_samples;
	
	//if (g_luminance_prefilter_mip_indx == 0u) {
	//	samples_count = 1u; // first sample always points in render_cubemap_dir, so with 1 sample the filter operation is identity
	//}
	
	#if 1
	v3	accum = v3(0);
	flt	accum_weight = 0;
	
	for (ui i=0u; i<samples_count; ++i) {
		
		v2	Xi = hammersley(i, samples_count);
		
		v3	half_ = importance_sample_GGX(Xi, roughness, norm);
		v3	light =	reflect(-view, half_);
		
		flt dln =	dot(light, norm);
		flt dhn =	dot(half_, norm);
		flt dhv =	dhn; // norm == view
		
		if (dln <= 0.0) {
			continue;
		}
		
		#if 1
		flt mip;
		{ // Why does this work so well now, i remember this fix making my lower prefilter totally blurry
			flt	D = distribution_GGX(dhn, roughness);
			flt	pdf = ((D*dhn) / (4.0 * dhv)) +0.0001;
			
			flt	res = flt(g_source_cubemap_res);
			flt	sa_texel = (4.0 * PI) / (6.0 * res*res);
			flt	sa_sample = 1.0 / (flt(samples_count) * pdf);
			
			mip = roughness == 0.0 ? 0.0 : 0.5 * log2(sa_sample / sa_texel);
		}
		
		v3	env_luminance_sample = textureLod(env_luminance_tex, light, mip).rgb;
		#else
		v3	env_luminance_sample = texture(env_luminance_tex, light).rgb;
		#endif
		
		accum += env_luminance_sample * v3(dln);
		accum_weight += dln;
		
	}
	
	return accum / v3(accum_weight);
	#else // Visulalize sample directions
	flt	least_dist = +10000000000000000000000000.0; // +infinity
	
	for (ui i=0u; i<samples_count; ++i) {
		
		v2	Xi = hammersley(i, samples_count);
		
		flt	a = roughness*roughness;
		
		flt	phi = Xi.x * DEG_360;
		
		flt	cos_theta = sqrt( (1.0 -Xi.y) / (1.0 +(a*a -1)*Xi.y) );
		flt	sin_theta = sqrt(1.0 -cos_theta*cos_theta);
		
		v3	half_ = v3(	sin_theta * cos(phi),
						sin_theta * sin(phi),
						cos_theta );
		
		flt dist = (1.0 -dot(half_, norm)) * (1024*64);
		
		least_dist = min(least_dist, dist);
	}
	
	return v3(least_dist);
	#endif
}

v3 convolve_illuminance (v3 interp_render_cubemap_dir) {
	
	m3 tang_to_env;
	{ // tangent space for convolution
		v3 z = normalize(interp_render_cubemap_dir);
		
		v3 up = normalize(v3(0,0,1));
		
		v3 x = normalize(cross(z, up)); // should this not in theory be able to return a NaN ?
		v3 y = cross(z, x);
		
		tang_to_env = m3( x, y, z );
	}
	
	const si ELEV_SAMPLES_COUNT =		32;
	const si AZIM_SAMPLES_COUNT =		ELEV_SAMPLES_COUNT * 4;
	const si TOTAL_SAMPLES_COUNT =		ELEV_SAMPLES_COUNT * AZIM_SAMPLES_COUNT;
	
	v3 accum = v3(0);
	
	for (si elev_i=0; elev_i<ELEV_SAMPLES_COUNT; ++elev_i) {
		
		flt theta = ((flt(elev_i) +0.5) / flt(ELEV_SAMPLES_COUNT)) * DEG_90;
		flt st = sin(theta);
		flt ct = cos(theta);
		
		for (si azim_i=0; azim_i<AZIM_SAMPLES_COUNT; ++azim_i) {
			
			flt phi = (flt(azim_i) / flt(AZIM_SAMPLES_COUNT)) * DEG_360;
			flt sp = sin(phi);
			flt cp = cos(phi);
			
			v3 tang_dir = v3(sp * st, cp * st, ct);
			
			v3 sample_dir = tang_to_env * tang_dir;
			
			accum += texture(env_luminance_tex, sample_dir).rgb * ct * st;
		}
		
	}
	
	v3 illuminance = (PI * accum) / v3(TOTAL_SAMPLES_COUNT);
	return illuminance;
}
#elif PBR_PRERENDER == 2
v2 integrate_brdf (flt dvn, flt roughness) {
	
	v3	norm = v3(0, 0, 1);
	v3	view = v3(	sqrt(1.0 -dvn*dvn), // sin from cos view-normal
					0,
					dvn );
	
	const ui SAMPLES_COUNT = ui(4096);
	
	flt	accum_a = 0;
	flt	accum_b = 0;
	
	for (ui i=0u; i<SAMPLES_COUNT; ++i) {
		
		v2	Xi = hammersley(i, SAMPLES_COUNT);
		
		v3	half_ = importance_sample_GGX(Xi, roughness, norm);
		v3	light =	reflect(-view, half_);
		
		flt dln =	dot(light, norm);
		flt dhn =	dot(half_, norm);
		flt dvh =	dot(view, half_);
		
		if (dln <= 0.0) {
			continue;
		}
		
		flt	G = geometry_smith_IBL(roughness, dvn, dln);
		flt	G_vis = (G * dvh) / (dhn * dvn);
		flt	Fc = pow(1.0 -dvh, 5.0);
		
		accum_a += (1.0 -Fc) * G_vis;
		accum_b += Fc * G_vis;
		
	}
	
	flt	a = accum_a / flt(SAMPLES_COUNT);
	flt	b = accum_b / flt(SAMPLES_COUNT);
	return v2(a, b);
}
#endif

#endif
