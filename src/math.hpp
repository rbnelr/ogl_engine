
namespace math {

/*
	Coordinate system convetions:
	X - right/east,		red, right hand is primary
	Y - forward/north,	green
	Z - up,				blue, least important coordinate, discribes height in maps etc., x & y are like coordinates in a map
*/

#define PI				3.1415926535897932384626433832795f
#define TWO_PI			6.283185307179586476925286766559f
#define PId				3.1415926535897932384626433832795
#define TWO_PId			6.283185307179586476925286766559

#define QNAN			fp::qnan<f32>()
#define QNANd			fp::qnan<f64>()

#define DEG_TO_RADd		(TWO_PId / 360.0)
#define DEG_TO_RAD		((f32)DEG_TO_RADd)
#define RAD_TO_DEGd		(360.0 / TWO_PId)
#define RAD_TO_DEG		((f32)RAD_TO_DEGd)

// Units which always convert to the internal representation (which is radians)
DECL constexpr  f32 deg (f32 deg) {
	return deg * DEG_TO_RAD;
}
DECL constexpr  f64 deg (f64 deg) {
	return deg * DEG_TO_RADd;
}
DECL constexpr  f32 rad (f32 rad) {
	return rad;
}
DECL constexpr  f64 rad (f64 rad) {
	return rad;
}

template <typename T>	DECL constexpr tv2<T> deg (tv2<T> vp deg) {		return deg * tv2<T>(T(DEG_TO_RADd)); }
template <typename T>	DECL constexpr tv3<T> deg (tv3<T> vp deg) {		return deg * tv3<T>(T(DEG_TO_RADd)); }
template <typename T>	DECL constexpr tv4<T> deg (tv4<T> vp deg) {		return deg * tv4<T>(T(DEG_TO_RADd)); }
template <typename T>	DECL constexpr tv2<T> rad (tv2<T> vp rad) {		return rad; }
template <typename T>	DECL constexpr tv3<T> rad (tv3<T> vp rad) {		return rad; }
template <typename T>	DECL constexpr tv4<T> rad (tv4<T> vp rad) {		return rad; }

// Conversions
DECL constexpr  f64 to_deg (f64 rad) {
	return rad * RAD_TO_DEGd;
}
DECL constexpr  f32 to_deg (f32 rad) {
	return rad * RAD_TO_DEG;
}
DECL constexpr  f32 to_rad (f32 deg) {
	return deg * DEG_TO_RAD;
}
DECL constexpr  f64 to_rad (f64 deg) {
	return deg * DEG_TO_RADd;
}

template <typename T>	DECL constexpr tv2<T> to_deg (tv2<T> vp rad) {	return rad * tv2<T>(T(RAD_TO_DEGd)); }
template <typename T>	DECL constexpr tv3<T> to_deg (tv3<T> vp rad) {	return rad * tv3<T>(T(RAD_TO_DEGd)); }
template <typename T>	DECL constexpr tv4<T> to_deg (tv4<T> vp rad) {	return rad * tv4<T>(T(RAD_TO_DEGd)); }
template <typename T>	DECL constexpr tv2<T> to_rad (tv2<T> vp deg) {	return deg * tv2<T>(T(DEG_TO_RADd)); }
template <typename T>	DECL constexpr tv3<T> to_rad (tv3<T> vp deg) {	return deg * tv3<T>(T(DEG_TO_RADd)); }
template <typename T>	DECL constexpr tv4<T> to_rad (tv4<T> vp deg) {	return deg * tv4<T>(T(DEG_TO_RADd)); }

DECL f32 mod_pn_180deg (f32 ang) { // mod float into -/+ 180deg range
	return fp::proper_mod(ang +deg(180.0f), deg(360.0f)) -deg(180.0f);
}

template <typename T>	DECL T to_linear (T srgb) {
	if (srgb <= T(0.0404482362771082)) {
		return srgb * T(1/12.92);
	} else {
		return fp::pow( (srgb +T(0.055)) * T(1/1.055), T(2.4) );
	}
}
template <typename T>	DECL T to_srgb (T linear) {
	if (linear <= T(0.00313066844250063)) {
		return linear * T(12.92);
	} else {
		return ( T(1.055) * fp::pow(linear, T(1/2.4)) ) -T(0.055);
	}
}

template <typename T> DECL constexpr tv3<T> to_linear (tv3<T> srgb) {
	return tv3<T>( to_linear(srgb.x), to_linear(srgb.y), to_linear(srgb.z) );
}
template <typename T> DECL constexpr tv3<T> to_srgb (tv3<T> linear) {
	return tv3<T>( to_srgb(linear.x), to_srgb(linear.y), to_srgb(linear.z) );
}

template <typename T>	DECL constexpr tv3<T> col (T x, T y, T z) {		return tv3<T>(x,y,z); }
template <typename T>	DECL constexpr tv3<T> srgb (T x, T y, T z) {	return to_linear(tv3<T>(x,y,z) * tv3<T>(T(1.0/255.0))); }
template <typename T>	DECL constexpr tv3<T> col (T all) {				return col(all,all,all); }
template <typename T>	DECL constexpr tv3<T> srgb (T all) {			return srgb(all,all,all); }

DECL m3 scale_3 (v3 vp scale) {
	m3 ret = m3::zero();
	ret.row(0, 0, scale.x);
	ret.row(1, 1, scale.y);
	ret.row(2, 2, scale.z);
	return ret;
}
DECL m3 rotate_X3 (f32 angle) {
	auto t = fp::sincos(angle);
	m3 ret = m3::row(	1,		0,		0,
						0,		+t.c,	-t.s,
						0,		+t.s,	+t.c );
	return ret;
}
DECL m3 rotate_Y3 (f32 angle) {
	auto t = fp::sincos(angle);
	m3 ret = m3::row(	+t.c,	0,		+t.s,
						0,		1,		0,
						-t.s,	0,		+t.c );
	return ret;
}
DECL m3 rotate_Z3 (f32 angle) {
	auto t = fp::sincos(angle);
	m3 ret = m3::row(	+t.c,	-t.s,	0,
						+t.s,	+t.c,	0,
						0,		0,		1 );
	return ret;
}

DECL m4 translate_4 (v3 vp v) {
	m4 ret = m4::ident();
	ret.column(3, v4(v, 1));
	return ret;
}
DECL m4 scale_4 (v3 vp scale) {
	m4 ret = m4::zero();
	ret.row(0, 0, scale.x);
	ret.row(1, 1, scale.y);
	ret.row(2, 2, scale.z);
	ret.row(3, 3, 1);
	return ret;
}
DECL m4 rotate_X4 (f32 angle) {
	auto t = fp::sincos(angle);
	m4 ret = m4::row(	1,		0,		0,		0,
						0,		+t.c,	-t.s,	0,
						0,		+t.s,	+t.c,	0,
						0,		0,		0,		1 );
	return ret;
}
DECL m4 rotate_Y4 (f32 angle) {
	auto t = fp::sincos(angle);
	m4 ret = m4::row(	+t.c,	0,		+t.s,	0,
						0,		1,		0,		0,
						-t.s,	0,		+t.c,	0,
						0,		0,		0,		1 );
	return ret;
}
DECL m4 rotate_Z4 (f32 angle) {
	auto t = fp::sincos(angle);
	m4 ret = m4::row(	+t.c,	-t.s,	0,		0,
						+t.s,	+t.c,	0,		0,
						0,		0,		1,		0,
						0,		0,		0,		1 );
	return ret;
}

DECL hm translate_h (v3 vp v) {
	#if 0
	return hm::row(	1, 0, 0, v.x,
					0, 1, 0, v.y,
					0, 0, 1, v.z);
	#else // Better asm
	hm ret;
	ret.arr[0].x = 1;
	ret.arr[0].y = 0;
	ret.arr[0].z = 0;
	ret.arr[1].x = 0;
	ret.arr[1].y = 1;
	ret.arr[1].z = 0;
	ret.arr[2].x = 0;
	ret.arr[2].y = 0;
	ret.arr[2].z = 1;
	ret.arr[3].x = v.x;
	ret.arr[3].y = v.y;
	ret.arr[3].z = v.z;
	return ret;
	#endif
}
DECL hm scale_h (v3 vp scale) {
	hm ret = hm::ident();
	ret.row(0, 0, scale.x);
	ret.row(1, 1, scale.y);
	ret.row(2, 2, scale.z);
	return ret;
}
DECL hm rotate_x (f32 angle) {
	auto t = fp::sincos(angle);
	return hm::row(	1,		0,		0,		0,
					0,		+t.c,	-t.s,	0,
					0,		+t.s,	+t.c,	0 );
}
DECL hm rotate_y (f32 angle) {
	auto t = fp::sincos(angle);
	return hm::row(	+t.c,	0,		+t.s,	0,
					0,		1,		0,		0,
					-t.s,	0,		+t.c,	0 );
}
DECL hm rotate_z (f32 angle) {
	auto t = fp::sincos(angle);
	return hm::row(	+t.c,	-t.s,	0,		0,
					+t.s,	+t.c,	0,		0,
					0,		0,		1,		0 );
}

DECL quat rotate_quat_axis (v3 vp axis, f32 angle) {
	auto t = fp::sincos(angle / 2.0f);
	return quat( axis * v3(t.s), t.c );
}
DECL quat rotate_XQuat (f32 angle) {
	auto t = fp::sincos(angle / 2.0f);
	return quat( v3(t.s, 0, 0), t.c );
}
DECL quat rotate_YQuat (f32 angle) {
	auto t = fp::sincos(angle / 2.0f);
	return quat( v3(0, t.s, 0), t.c );
}
DECL quat rotate_ZQuat (f32 angle) {
	auto t = fp::sincos(angle / 2.0f);
	return quat( v3(0, 0, t.s), t.c );
}

constexpr f32 _90deg_arr[5] = { 0, +1, 0, -1, 0 };
DECL fp::SC _90deg_getSC (si multiple) {
	fp::SC ret;
	
	multiple = multiple & 0b11;
	
	ret.s = _90deg_arr[multiple];
	ret.c = _90deg_arr[multiple +1];
	return ret;
};
DECL m3 rotate_X_90deg (si multiple) {
	auto g = _90deg_getSC(multiple);
	m3 ret = m3::row(	1,		0,		0,
						0,		+g.c,	-g.s,
						0,		+g.s,	+g.c );
	return ret;
};
DECL m3 rotate_Y_90deg (si multiple) {
	auto g = _90deg_getSC(multiple);
	m3 ret = m3::row(	+g.c,	0,		+g.s,
						0,		1,		0,
						-g.s,	0,		+g.c );
	return ret;
};
DECL m3 rotate_Z_90deg (si multiple) {
	auto g = _90deg_getSC(multiple);
	m3 ret = m3::row(	+g.c,	-g.s,	0,
						+g.s,	+g.c,	0,
						0,		0,		1 );
	return ret;
};

////
DECL m3 aer_world_to_cam_m (v3 vp aer) {
	// 0,0,0 points down in world (this is how camera space works, since we have z-up instead of y-up),
	//  pitch range: [0,180]
	// note the reverse order of rotations (still zxz because zxz mirrored is zxz)
	#if 0
	m3 ret;
	ret =	rotate_Z3(aer.z);
	ret *=	rotate_X3(aer.y);
	ret *=	rotate_Z3(aer.x);
	return ret;
	#else
	v3 aer_ = aer;
	//aer_.z = deg(30.0f);
	
	//m3 test;
	//test =	rotationZ3(aer_.z);
	//test *=	rotationX3(aer_.y);
	//test *=	rotationZ3(aer_.x);
	
	m3 ret = mat_op::compute_euler_angle_zxz_m3(v3(aer_.z, aer_.y, aer_.x));
	
	//print("test:\n%\n%\n", test, ret);
	
	return ret;
	#endif
}
DECL m3 aer_horizontal_m (v3 vp aer) { // 0,0,0 points +y in world, pitch range: [-90,+90]
	#if 0
	m3 ret;
	ret =	rotationZ3(aer.x);
	ret *=	rotationX3(aer.y);
	ret *=	rotationY3(aer.z);
	return ret;
	#else
	
	//m3 test;
	//test =	rotationZ3(aer.x);
	//test *=  rotationX3(aer.y);
	//test *=	rotationY3(aer.z);
	
	m3 ret = mat_op::compute_euler_angle_zxy_m3(aer);
	
	//print("test:\n%\n%\n", test, ret);
	
	return ret;
	#endif
}
DECL quat aer_camera_q (v3 vp aer) {
	#if 0
	quat ret;
	ret =	rotationZQuat(aer.z);
	ret *=	rotationXQuat(aer.y);
	ret *=	rotationZQuat(aer.x);
	return ret;
	#else // easily optimized by removing zero products
	return quat_op::compute_euler_angle_zxz_q(v3(aer.z, aer.y, aer.x));
	#endif
}
DECL quat aer_horizontal_q_from_euler (v3 vp aer) {
	quat ret;
	ret =	rotate_ZQuat(aer.x);
	ret *=	rotate_XQuat(aer.y);
	ret *=	rotate_YQuat(aer.z);
	return ret;
}
DECL v3 aer_horizontal_q_to_euler (quat vp q) {
	
	// Implementation derived with help of this paper: https://www.geometrictools.com/Documentation/EulerAngles.pdf
	//  and the modified/optimized result matches the form found at https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
	// This does have drift when elev is close to +/- 90deg (noticable >89.5deg) (some angle value drives over the course of quat->euler->quat)
	// And obviously with euler angles there are singularities where angles lose all precision (and so their information)
	//  and at the singularity can possibly also become NaN if not accounted for (in this impl asin() has it's imput clamped and atan2 does not seem to return NaN (it seems like it returns 0 ie. garbage but non-NaN data))
	
#if 0
	m3 m = conv_to_m3(q);
	
	v3 ret;
	#if 0	// These cases seem to be completely unnecessary,
			//  since they only seem to prevent out of range errors (nan) in the asin function, which can simply be handled with a clamp since values should only ever be epsilon over/under +/-1
			//  atan2 seems to always behave properly
	if (		m.arr[1].z <= -1.0f ) {
		print("  if 1");
		ret.x = -fp::atan2(m.arr[2].x, m.arr[0].x);
		ret.y = -deg(90.0f);
		ret.z = 0.0f;
	} else if (	m.arr[1].z >= -1.0f ) {
		print("  if 2");
		ret.x = +fp::atan2(m.arr[2].x, m.arr[0].x);
		ret.y = +deg(90.0f);
		ret.z = 0.0f;
	} else {
		print("  else");
	#endif
	
		ret.x = fp::atan2(-m.arr[1].x, m.arr[1].y);
		ret.y = fp::asin(m.arr[1].z);
		ret.z = fp::atan2(-m.arr[0].z, m.arr[2].z);
	
	#if 0
	}
	#endif
	return ret;
#else
	
	f32 zw = q.z * q.w;
	f32 xy = q.x * q.y;
	f32 yz = q.y * q.z;
	f32 xw = q.x * q.w;
	f32 yw = q.y * q.w;
	f32 xz = q.x * q.z;
	f32 xx = q.x * q.x;
	f32 yy = q.y * q.y;
	f32 zz = q.z * q.z;
	
	v3 ret;
	ret.x = fp::atan2(2.0f * (zw -xy), 1.0f -( 2.0f * (xx +zz) ));
	ret.y = fp::asin(fp::clamp(2.0f * (yz +xw), -1.0f, +1.0f));
	ret.z = fp::atan2(2.0f * (yw -xz), 1.0f -( 2.0f * (xx +yy) ));
	return ret;
	
#endif
}
DECL v3 aer_horizontal_q_to_euler_geom (quat vp q) {
	
	// Implementation derived with help of this paper: http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/quat_2_euler_paper_ver2-1.pdf
	// This geometric conversions seems a bit more stable when approaching the singularities (gibal lock)
	//   (It does still drift when elev is close to +/- 90deg (noticable >89.5deg))
	//  possibly because every the second angle is calculated from the first and the third from the first two
	//  although in that case this still has the problem of precision loss (possibly even nan?) in on the first angle (atan2)
	
	#if 0
	quat Qg = q;
	
	v3 V3 = v3(0,1,0);
	
	v3 V3rot = Qg * V3;
	
	f32 ang1 = fp::atan2(-V3rot.x, +V3rot.y); // does not ever seen to return nan, so just let it return noisy/garbage angle for now
	f32 ang2 = fp::asin(clamp(+V3rot.z, -1.0f, +1.0f));
	
	quat Q1 = rotate_ZQuat(ang1);
	quat Q2 = rotate_XQuat(ang2);
	
	quat Q12 = Q1 * Q2;
	
	v3 V3n = v3(0,0,1);
	
	v3 V3n12 = Q12 * V3n;
	v3 V3nG = Qg * V3n;
	
	f32 cos_ang = dot(V3n12, V3nG);
	f32 mag = fp::acos(sse::clamp(cos_ang, -1.0f, +1.0f));
	
	v3 Vc = cross(V3n12, V3nG);
	f32 m = dot(Vc, V3rot);
	
	f32 ang3 = fp::set_sign(mag, fp::get_sign(m));
	
	return v3(ang1, ang2, ang3);
	
	#else
	// ret.x = (l.y * r.z) -(l.z * r.y);
	// ret.y = (l.z * r.x) -(l.x * r.z);
	// ret.z = (l.x * r.y) -(l.y * r.x);
	// 
	// 
	// tv3<T> qv = tv3<T>(q.x, q.y, q.z);
	// tv3<T> uv = vec_op::cross(qv, v);
	// tv3<T> uuv = vec_op::cross(qv, uv);
	// 
	// return v +::operator*( (::operator*(uv, tv3<T>(q.w)) +uuv), tv3<T>(2) );
	
	v3 V3rot;
	V3rot.x = ( ((q.y * q.x) -(q.z * q.w)) *  2.0f );
	V3rot.y = ( ((q.z * q.z) +(q.x * q.x)) * -2.0f ) +1.0f;
	V3rot.z = ( ((q.x * q.w) +(q.y * q.z)) *  2.0f );
	
	v3 aer;
	aer.x = fp::atan2(-V3rot.x, +V3rot.y);
	aer.y = fp::asin(clamp(+V3rot.z, -1.0f, +1.0f));
	
	auto t1 = fp::sincos(aer.x / 2.0f);
	auto t2 = fp::sincos(aer.y / 2.0f);
	
	v3 V3n12;
	v3 V3nG;
	{
		
		f32 t1s1c = t1.s * t1.c;
		f32 t1s1s = t1.s * t1.s;
		f32 t1c1c = t1.c * t1.c;
		
		f32 t2s2c = t2.s * t2.c;
		f32 t2s2s = t2.s * t2.s;
		f32 t2c2s = t2.c * t2.s;
		
		V3n12.x = ( ((t1s1c * t2s2c) +(t1s1c * t2c2s)) *  2.0f );
		V3n12.y = ( ((t1s1s * t2s2c) -(t1c1c * t2c2s)) *  2.0f );
		V3n12.z = ( ((t1c1c * t2s2s) +(t1s1s * t2s2s)) * -2.0f ) +1.0f;
		
		
		V3nG.x = ( ((q.z * q.x) +(q.y * q.w)) *  2.0f );
		V3nG.y = ( ((q.z * q.y) -(q.x * q.w)) *  2.0f );
		V3nG.z = ( ((q.x * q.x) +(q.y * q.y)) * -2.0f ) +1.0f;
		
	}
	
	f32 cos_ang = dot(V3n12, V3nG);
	f32 mag = fp::acos(fp::clamp(cos_ang, -1.0f, +1.0f));
	
	v3 Vc = cross(V3n12, V3nG);
	f32 m = dot(Vc, V3rot);
	
	aer.z = fp::set_sign(mag, fp::get_sign(m));
	
	return aer;
	#endif
}

////
DECL hm inverse_transl_ori (hm mp m) { // faster inverse for rotate->translate matricies
	return hm::ident().m3(transpose(m.m3())) * translate_h(-m.column(3).xyz());
}

struct Ray {
	v3	pos;
	v3	dir; // not normalized
};
DECL Ray target_ray_from_screen_pos (v2 vp window_pos, v2 vp window_res, v2 vp cam_frust_scale, f32 clip_near) { // result in cam space
	v2 temp = window_pos / window_res;
	temp = (temp * v2(2.0f)) -v2(1.0f);
	temp *= cam_frust_scale;
	Ray ret;
	ret.dir = v3(temp, -1.0f);
	ret.pos = ret.dir * v3(clip_near); // ray.pos is on near clip plane
	return ret;
}
DECL Ray target_ray_from_screen_pos (v2 vp window_pos, v2 vp window_res, v2 vp cam_frust_scale, f32 clip_near, hm mp cam_to_space) {
	Ray ret = target_ray_from_screen_pos(window_pos, window_res, cam_frust_scale, clip_near);
	ret.pos = (cam_to_space * hv(ret.pos)).xyz();
	ret.dir = cam_to_space.m3() * ret.dir;
	return ret;
}
DECL v2 project_point_onto_screen (v3 vp cam, m4 mp camera_to_clip, v2 res) { 
	v4 clip = camera_to_clip * v4(cam, 1.0f);
	v2 ndc = clip.xy() / v2(clip.w);
	v2 screen = ((ndc +v2(1.0f)) * v2(0.5f)) * res;
	return screen;
}
DECL f32 sphere_ray_intersect (v3 vp sphere_pos, f32 sphere_radius, v3 vp ray_pos, v3 vp ray_dir) {
	v3 sphere_pos_rel = sphere_pos -ray_pos;
	f32 dot_sphere_onto_ray = dot(sphere_pos_rel, ray_dir);
	if (dot_sphere_onto_ray >= 0.0f) {
		
		v3 closest_pos_rel = v3(dot_sphere_onto_ray) * ray_dir;
		v3 v_to_sphere = sphere_pos_rel -closest_pos_rel;
		f32 dist_to_sphere = length(v_to_sphere);
		if (dist_to_sphere <= sphere_radius) {
			
			f32 dist_into_sphere = fp::sqrt(SQUARED(sphere_radius) -SQUARED(dist_to_sphere));
			f32 dist_to_first_intersect = dot_sphere_onto_ray -dist_into_sphere;
			return dist_to_first_intersect;
		}
	}
	return fp::inf<f32>();
}
DECL bool ray_line_closest_intersect (v3 vp posRay, v3 vp dirRay, v3 vp posLine, v3 vp dirLine,
		v3* outIntersect) {
	
	v3 crossDir = cross(dirRay, dirLine);
	f32 divisor = length_sqr(crossDir);
	
	if (divisor == 0.0f) {
		// target ray and axis line are exactly parallel
		return false;
	}
	
	f32 invDivisor = 1.0f / divisor;
	v3 diffPos = posLine -posRay;
	
	m3 m = m3::column( diffPos, dirLine, crossDir );
	f32 tRay = determinant(m);
	tRay *= invDivisor;
	
	if (tRay < 0.0f) {
		// target ray points away from axis line (intersection would be backwards on the ray (behind the camera))
		return false;
	}
	
	m.column(1, dirRay);
	f32 tLine = determinant(m);
	tLine *= invDivisor;
	
	*outIntersect = posLine +(dirLine * v3(tLine));
	return true;
}

struct Intersect {
	f32	dist;
	v3	pos;
};

DECL Intersect ray_triangle_intersect (v3 vp tri_a, v3 vp tri_b, v3 vp tri_c, v3 vp ray_pos, v3 vp ray_dir) {
	// Copied from http://geomalgorithms.com/a06-_intersect-2.html
	// ray_dir can be any length vec
	
	v3 u = tri_b -tri_a; // triangle edge 1
	v3 v = tri_c -tri_a; // triangle edge 2
	v3 n = cross(u, v); // triangle normal
	
	f32 proj_div = dot(n, ray_dir);
	if (proj_div == 0.0f) { // TODO: EPSILON?
		return { fp::inf<f32>(), v3(fp::qnan<f32>()) }; // ray parallel to triangle
	}
	
	f32 proj_num = dot(n, tri_a -ray_pos);
	f32 ri = proj_num / proj_div;
	if (ri < 0.0f) {
		return { fp::inf<f32>(), v3(fp::qnan<f32>()) }; // intersection on backwards part of ray
	}
	
	v3 p = ray_pos +ray_dir * v3(ri); // intersection point on triangle plane
	
	v3 w = p -tri_a;
	
	f32 uv = dot(u, v);
	f32 uu = dot(u, u);
	f32 vv = dot(v, v);
	f32 wu = dot(w, u);
	f32 wv = dot(w, v);
	
	f32 inv_div = 1.0f / (uv*uv -uu*vv); // should never be zero for non-degenerate triangles
	
	f32 s = (uv*wv -vv*wu) * inv_div;
	f32 t = (uv*wu -uu*wv) * inv_div;
	
	if (s < 0.0f || t < 0.0f || (s +t) > 1.0f) {
		return { fp::inf<f32>(), v3(fp::qnan<f32>()) }; // Intersection outside of triangle
	} 
	
	return { ri, p };
}

DECL v3 triangle_calc_barycentric_coords (v3 vp p, v3 vp tri_a, v3 vp tri_b, v3 vp tri_c) {
	
	v3 u = tri_b -tri_a; // triangle edge 1
	v3 v = tri_c -tri_a; // triangle edge 2
	
	v3 w = p -tri_a;
	
	f32 uv = dot(u, v);
	f32 uu = dot(u, u);
	f32 vv = dot(v, v);
	f32 wu = dot(w, u);
	f32 wv = dot(w, v);
	
	f32 inv_div = 1.0f / (uv*uv -uu*vv); // should never be zero for non-degenerate triangles
	
	v3 ret;
	ret.y = (uv*wv -vv*wu) * inv_div;
	ret.z = (uv*wu -uu*wv) * inv_div;
	ret.x = 1.0f -ret.y -ret.z;
	return ret;
}

////
struct Line3 {
	v3	a, b;
};

union Box_Edges {
	struct {
		Line3	lll_hll;
		Line3	hll_hhl;
		Line3	hhl_lhl;
		Line3	lhl_lll;
		
		Line3	lll_llh;
		Line3	hll_hlh;
		Line3	hhl_hhh;
		Line3	lhl_lhh;
		
		Line3	llh_hlh;
		Line3	hlh_hhh;
		Line3	hhh_lhh;
		Line3	lhh_llh;
	};
	Line3		arr[12];
};
union Box_Corners { // l=low h=high
	struct {
		v3		lll;
		v3		hll;
		v3		hhl;
		v3		lhl;
		
		v3		llh;
		v3		hlh;
		v3		hhh;
		v3		lhh;
	};
	v3			arr[8];
	
	Box_Edges box_edges () const {
		Box_Edges ret;
		ret.lll_hll = {lll, hll};
		ret.hll_hhl = {hll, hhl};
		ret.hhl_lhl = {hhl, lhl};
		ret.lhl_lll = {lhl, lll};
		
		ret.lll_llh = {lll, llh};
		ret.hll_hlh = {hll, hlh};
		ret.hhl_hhh = {hhl, hhh};
		ret.lhl_lhh = {lhl, lhh};
		
		ret.llh_hlh = {llh, hlh};
		ret.hlh_hhh = {hlh, hhh};
		ret.hhh_lhh = {hhh, lhh};
		ret.lhh_llh = {lhh, llh};
		return ret;
	}
};

union AABB {
	// l -> low
	// h -> high
	// low <= high
	struct {
		f32	xl, xh;
		f32	yl, yh;
		f32	zl, zh;
	};
	f32	arr[6];
	
	static constexpr AABB inf () { // initialize to infinite values so that doing MIN() & MAX() in a loop will get us the AABB
		return { {	+fp::inf<f32>(), -fp::inf<f32>(),
					+fp::inf<f32>(), -fp::inf<f32>(),
					+fp::inf<f32>(), -fp::inf<f32>() } };
	}
	bool is_inf () const {
		bool ret = !std::isfinite(xl);
		#if DEBUG
		if (ret) {
			for (ui i=0; i<6; ++i) {
				assert(!std::isnan(arr[i]) && !std::isfinite(arr[i]));
			}
		}
		#endif
		return ret;
	}
	
	AABB& minmax (v3 vp v) {
		xl = MIN(xl, v.x);
		xh = MAX(xh, v.x);
		yl = MIN(yl, v.y);
		yh = MAX(yh, v.y);
		zl = MIN(zl, v.z);
		zh = MAX(zh, v.z);
		return *this;
	}
	AABB& minmax (Box_Corners cr box) {
		for (u32 i=0; i<8; ++i) {
			minmax(box.arr[i]);
		}
		return *this;
	}
	AABB& minmax (AABB cr aabb) {
		minmax(aabb.box_corners());
		return *this;
	}
	
	static AABB from_obb (Box_Corners cr obb) {
		AABB ret = inf();
		ret.minmax(obb);
		return ret;
	}
	
	Box_Corners box_corners () const {
		Box_Corners ret;
		ret.lll =	v3(xl,yl,zl);
		ret.hll =	v3(xh,yl,zl);
		ret.hhl =	v3(xh,yh,zl);
		ret.lhl =	v3(xl,yh,zl);
		
		ret.llh	=	v3(xl,yl,zh);
		ret.hlh	=	v3(xh,yl,zh);
		ret.hhh	=	v3(xh,yh,zh);
		ret.lhh	=	v3(xl,yh,zh);
		return ret;
	}
	
	Box_Edges box_edges () const {
		return box_corners().box_edges();
	}
	
};

} // namespace math

math::Box_Corners operator* (m3 mp m, math::Box_Corners cr b) {
	math::Box_Corners ret;
	for (ui i=0; i<8; ++i) {
		ret.arr[i] = m * b.arr[i];
	}
	return ret;
}
math::Box_Corners operator* (hm mp m, math::Box_Corners cr b) {
	math::Box_Corners ret;
	for (ui i=0; i<8; ++i) {
		ret.arr[i] = (m * hv(b.arr[i])).xyz();
	}
	return ret;
}

math::Box_Corners operator+ (math::Box_Corners cr b, v3 vp v) {
	math::Box_Corners ret;
	for (ui i=0; i<8; ++i) {
		ret.arr[i] = b.arr[i] +v;
	}
	return ret;
}
