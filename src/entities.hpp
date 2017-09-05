
namespace entities_n {
	enum entity_tag : u32 {
		ET_ROOT=0,
		ET_MESH,
		ET_MESH_NANOSUIT,
		ET_MATERIAL_SHOWCASE_GRID,
		ET_GROUP,
		ET_LIGHT,
		ET_SCENE,
	};
	enum entity_flag : u32 {
		EF_HAS_MESHES=	0b01,
	};
	DEFINE_ENUM_FLAG_OPS(entity_flag, u32)
	
	enum light_type_e : u32 {
		LT_DIRECTIONAL=0,
		LT_POINT,
		
		LIGHT_TYPES
	};
	enum light_flags_e : u32 {
		LF_DISABLED =	0b01,
		LF_SHADOW =		0b10,
	}; DEFINE_ENUM_FLAG_OPS(light_flags_e, u32)
	DECLD constexpr Enum_Member _LIGHT_FLAGS_E[] = {
		{ "DISABLED",	0,1 },
		{ "SHADOW",		1,1 },
	};
	#define LIGHT_FLAGS_E ::array<Enum_Member>{(Enum_Member*)_LIGHT_FLAGS_E, arrlenof<u32>(_LIGHT_FLAGS_E)}
	
	
	struct Textures_Generic {
		textures_e	albedo;
		textures_e	normal;
		textures_e	roughness;
		textures_e	metallic;
	};
	struct Textures_Nanosuit {
		textures_e	diffuse_emissive;
		textures_e	normal;
		textures_e	specular_roughness;
	};
	
	DECLD v2u32		material_showcase_grid_steps;
	DECLD m2		material_showcase_grid_mat = m2::row(	+1, 0,
															 0,-1 );
	
	struct Entity {
		lstr			name;
		
		Entity*			next;
		Entity*			parent;
		Entity*			children;
		// Relative to parent space / children are in the space defined by these placement variables
		//  scale always first, orientation second and position last
		v3				pos;
		quat			ori;
		v3				scale;
		
		AABB			aabb_mesh_paren; // only valid if (eflags & EF_HAS_MESHES)
		AABB			aabb_mesh_world; // only valid if (eflags & EF_HAS_MESHES)
		
		entity_flag		eflags;
		entity_tag		tag;
	};
	
	struct eMesh_Base : public Entity {
		Mesh*			mesh;
		materials_e		material;
	};
	struct eMesh : public eMesh_Base {
		Textures_Generic	tex;
	};
	struct eMesh_Nanosuit : public eMesh_Base {
		Textures_Nanosuit	tex;
	};
	struct Material_Showcase_Grid : public Entity {
		Mesh*			mesh;
		v2				grid_offs;
	};
	struct Group : public Entity {
		
	};
	struct Light : public Entity {
		light_type_e	type;
		light_flags_e	flags;
		v3				luminance;
		
		Mesh*			mesh;
	};
	struct Scene : public Entity {
		bool			draw;
		dynarr<Light*>	lights;
	};
	struct Root : public Entity {
		constexpr Root (): Entity{
				"<root>", nullptr, nullptr, nullptr,
			v3(QNAN), quat(v3(QNAN), QNAN), v3(QNAN),
			{{QNAN,QNAN, QNAN,QNAN, QNAN,QNAN}},	{{QNAN,QNAN, QNAN,QNAN, QNAN,QNAN}},
			(entity_flag)0, ET_ROOT,
		} {}
	};
	
	union Entity_union {
		Entity					base;
		eMesh_Base				eMesh_Base;
		eMesh					eMesh;
		eMesh_Nanosuit			eMesh_Nanosuit;
		Material_Showcase_Grid	Material_Showcase_Grid;
		Group					Group;
		Light					Light;
		Scene					Scene;
		Root					Root;
		
		DECLM FORCEINLINE Entity_union () {}
	};
	
	hm me_to_parent (Entity const* e) {
		return translate_h(e->pos) * hm::ident().m3(conv_to_m3(e->ori)) * scale_h(e->scale);
	}
	hm parent_to_me (Entity const* e) {
		return scale_h(v3(1) / e->scale) * hm::ident().m3(conv_to_m3(inverse(e->ori))) * translate_h(-e->pos);
	}
	hm calc_me_to_world (Entity const* e) {
		hm ret = me_to_parent(e);
		e = e->parent;
		while (e->tag != ET_ROOT) {
			ret = me_to_parent(e) * ret;
			e = e->parent;
		}
		return ret;
	}
	hm calc_mat_me_world (Entity const* e, hm* out_world_to_me) {
		hm ret = me_to_parent(e);
		hm inv = parent_to_me(e);
		e = e->parent;
		while (e->tag != ET_ROOT) {
			ret = me_to_parent(e) * ret;
			inv = inv * parent_to_me(e);
			e = e->parent;
		}
		*out_world_to_me = inv;
		return ret;
	}
	hm calc_parent_to_world (Entity const* e) {
		hm ret;
		if (e->parent->tag != ET_ROOT) {
			e = e->parent;
			ret = me_to_parent(e);
			
			e = e->parent;
			while (e->tag != ET_ROOT) {
				ret = me_to_parent(e) * ret;
				e = e->parent;
			}
		} else {
			ret = hm::ident();
		}
		return ret;
	}
	hm calc_mat_parent_world (Entity const* e, hm* out_world_to_parent) {
		hm ret;
		hm inv;
		if (e->parent->tag != ET_ROOT) {
			e = e->parent;
			ret = me_to_parent(e);
			inv = parent_to_me(e);
			
			e = e->parent;
			while (e->tag != ET_ROOT) {
				ret = me_to_parent(e) * ret;
				inv = inv * parent_to_me(e);
				e = e->parent;
			}
		} else {
			ret = hm::ident();
			inv = hm::ident();
		}
		*out_world_to_parent = inv;
		return ret;
	}
	
	ui calc_depth (Entity const* e) {
		ui ret = 0;
		e = e->parent;
		while (e) { // root == level 0, this function can also be called on the root node
			++ret;
			e = e->parent;
		}
		return ret;
	}
	
	template <typename F>	void for_entity_subtree (Entity* e, F func) {
		func(e);
		
		e = e->children;
		while (e) {
			for_entity_subtree(e, func);
			
			e = e->next;
		}
	}
	template <typename F>	void for_entity_subtree_mat (Entity* e, hm mp parent_to_cam, hm mp cam_to_parent, F func) {
		hm mat = parent_to_cam * me_to_parent(e);
		hm inv = parent_to_me(e) * cam_to_parent;
		
		func(e, mat, inv, parent_to_cam);
		
		e = e->children;
		while (e) {
			for_entity_subtree_mat(e, mat, inv, func);
			
			e = e->next;
		}
	}
	
	DECL AABB calc_material_showcase_grid_aabb (Material_Showcase_Grid const* m) {
		
		assert(material_showcase_grid_steps.x >= 1 && material_showcase_grid_steps.y >= 1);
		
		AABB aabb_mesh = m->mesh->aabb;
		
		auto mat = translate_h(v3( material_showcase_grid_mat * (cast_v2<v2>(material_showcase_grid_steps -v2u32(1)) * m->grid_offs), 0.0f ));
		auto aabb_outter = mat * aabb_mesh.box_corners();
		
		return aabb_mesh.minmax(aabb_outter);
	}
	
};
using namespace entities_n;

struct Entities {
	
	Root					root; // init to root node data
	
	template <typename T, entity_tag TAG>
	T* make_entity (lstr cr name, v3 vp pos, quat vp ori, v3 vp scale=v3(1)) const {
		T* ret = working_stk.push<T>();
		ret->name =		name;
		ret->next =		nullptr;
		ret->parent =	nullptr;
		ret->children =	nullptr;
		
		ret->pos =		pos;
		ret->ori =		ori;
		ret->scale =	scale;
		
		ret->eflags =	(entity_flag)0;
		ret->tag =		TAG;
		
		//print(">>> Entity '%' at %\n", name, ret);
		return ret;
	}
	
	eMesh* mesh (lstr cr name, v3 vp pos, quat vp ori, lstr cr mesh_name,
			materials_e mat=MAT_WHITENESS,
			textures_e albedo=TEX_IDENT, textures_e norm=TEX_IDENT, textures_e roughness=TEX_IDENT, textures_e metallic=TEX_IDENT) {
		auto* ret = make_entity<eMesh, ET_MESH>(name, pos, ori);
		ret->mesh = meshes.get_mesh(mesh_name);
		ret->material = mat;
		ret->tex.albedo =		albedo;
		ret->tex.normal =		norm;
		ret->tex.roughness =	roughness;
		ret->tex.metallic =		metallic;
		return ret;
	}
	eMesh* mesh (lstr cr name, v3 vp pos, quat vp ori, v3 vp scale, lstr cr mesh_name,
			materials_e mat=MAT_WHITENESS,
			textures_e albedo=TEX_IDENT, textures_e norm=TEX_IDENT, textures_e roughness=TEX_IDENT, textures_e metallic=TEX_IDENT) {
		auto* ret = make_entity<eMesh, ET_MESH>(name, pos, ori, scale);
		ret->mesh = meshes.get_mesh(mesh_name);
		ret->material = mat;
		ret->tex.albedo =		albedo;
		ret->tex.normal =		norm;
		ret->tex.roughness =	roughness;
		ret->tex.metallic =		metallic;
		return ret;
	}
	
	eMesh_Nanosuit* mesh_nano (lstr cr name, v3 vp pos, quat vp ori, lstr cr mesh_name, materials_e mat,
			textures_e diff_emiss, textures_e norm, textures_e spec_rough) {
		auto* ret = make_entity<eMesh_Nanosuit, ET_MESH_NANOSUIT>(name, pos, ori, v3(1));
		ret->mesh = meshes.get_mesh(mesh_name);
		ret->material = mat;
		ret->tex.diffuse_emissive =		diff_emiss;
		ret->tex.normal =				norm;
		ret->tex.specular_roughness =	spec_rough;
		return ret;
	}
	
	Material_Showcase_Grid* material_showcase_grid (lstr cr name, v3 vp pos, quat vp ori, v3 vp scale,
			lstr cr mesh_name, v2 vp grid_offs) {
		auto* ret = make_entity<Material_Showcase_Grid, ET_MATERIAL_SHOWCASE_GRID>(name, pos, ori, scale);
		ret->mesh = meshes.get_mesh(mesh_name);
		ret->grid_offs = grid_offs;
		return ret;
	}
	
	Group* group (lstr cr name, v3 vp pos, quat vp ori) {
		auto* ret = make_entity<Group, ET_GROUP>(name, pos, ori);
		return ret;
	}
	
	Light* sun_lamp (lstr cr name, v3 vp pos, quat vp ori, light_flags_e flags, v3 vp luminance) {
		auto* ret = make_entity<Light, ET_LIGHT>(name, pos, ori);
		ret->type = LT_DIRECTIONAL;
		ret->flags = flags;
		ret->luminance = luminance;
		ret->mesh = meshes.get_mesh("sun_lamp.nouv_vcol");
		return ret;
	}
	Light* light_bulb (lstr cr name, v3 vp pos, light_flags_e flags, v3 vp luminance) {
		auto* ret = make_entity<Light, ET_LIGHT>(name, pos, quat::ident());
		ret->type = LT_POINT;
		ret->flags = flags;
		ret->luminance = luminance;
		ret->mesh = meshes.get_mesh("light_bulb.nouv_vcol");
		return ret;
	}
	
	Scene* scene (lstr cr name, v3 vp pos, quat vp ori, bool draw=true) {
		auto* ret = make_entity<Scene, ET_SCENE>(name, pos, ori);
		
		ret->draw = draw;
		ret->lights = ret->lights.alloc(8);
		
		return ret;
	}
	
	template <typename P, typename E, typename... Es>
	P* tree (P* parent, E* child0, Es... children) {
		Entity*	child[] = { child0, children... };
		
		parent->children = child[0];
		
		for (uptr i=0;;) {
			child[i]->parent = parent;
			
			if (++i == arrlenof(child)) break;
			
			child[i -1]->next = child[i];
		}
		
		assert(child[arrlenof(child) -1]->next == nullptr);
		
		return parent;
	}
	
	template <typename P, typename E, typename... Es>
	Scene* scene_tree (P* scn, E* child0, Es... children) {
		
		tree(scn, child0, children...);
		
		for_entity_subtree(scn,
			[&] (Entity const* e) {
				switch (e->tag) {
					case ET_LIGHT:	scn->lights.push((Light*)e);	break;
					default: {}
				}
			}
		);
		
		return scn;
	}
	
	void test_enumerate (Entity* e) {
		for_entity_subtree(e,
			[&] (Entity const* e) {
				if (e == &root) {
					print("%'%'\n", repeat("  ", calc_depth(e)), e->name);
				} else {
					print("%'%' % %\n", repeat("  ", calc_depth(e)), e->name, e->pos, e->ori);
				}
			}
		);
	}
	
	void test_enumerate_all () {
		print(">>> Entity enumeration:\n");
		test_enumerate(&root);
	}
	
	void test_entities () {
		
		auto ON = (light_flags_e)0;
		auto OFF = LF_DISABLED;
		auto SHAD = ON|LF_SHADOW;
		
		#define ROUGH(r) TEX_IDENT, TEX_IDENT, r, TEX_IDENT
		
		#define GROUND(w,h) mesh("ground", v3(0), quat::ident(), v3(w,h,1), "_unit_plane", MAT_GRASS);
		
		auto* scn = scene("shadow_test_0",
				v3(-6.19f, +12.07f, +1.00f), quat::ident());
			
			auto* gnd0 = GROUND(8,6);
			
			auto* lgh0 = sun_lamp("Test dir light",
					v3(+1.21f, -2.92f, +3.51f), quat(v3(+0.61f, +0.01f, +0.01f), +0.79f),
					SHAD,
					srgb(244,217,171) * col(2000));
			
			auto* msh0 = mesh("shadow_test_0",
					v3(+2.67f, +2.47f, +0.07f), quat(v3(+0.00f, -0.00f, -0.04), +1.00f),
					"shadow_test_0.nouv", MAT_ROUGH_MARBLE);
				
				auto* lgh1 = light_bulb("Test point light 1",
						v3(+0.9410f, +1.2415f, +1.1063f),
						ON, srgb(200,48,79) * col(100));
				auto* lgh2 = light_bulb("Test point light 2",
						v3(+1.0914f, +0.5582f, +1.3377f),
						ON, srgb(48,200,79) * col(100));
				auto* lgh3 = light_bulb("Test point light 3",
						v3(+0.3245f, +0.7575f, +1.0226f),
						ON, srgb(48,7,200) * col(100));
				
			auto* msh1 = mesh("Window_Pillar",
					v3(-3.21f, +0.00f, +0.16f), quat(v3(-0.00f, +0.00f, -1.00f), +0.08f),
					"window_pillar.nouv", MAT_ROUGH_MARBLE);
				
				auto* lgh4 = light_bulb("Torch light L",
						v3(-0.91969f, +0.610f, +1.880f),
						ON, srgb(240,142,77) * col(60));
				auto* lgh5 = light_bulb("Torch light R",
						v3(+0.91969f, +0.610f, +1.880f),
						ON, srgb(240,142,77) * col(60));
			
			auto* nano = group("Nanosuit",
					v3(+1.64f, +3.23f, +0.54f), quat(v3(+0.00f, +0.02f, +1.00f), +0.01f));
				
				auto* nano0 = mesh_nano("Torso",	v3(0), quat::ident(), "nano_suit/nanosuit_torso",	MAT_IDENTITY,
						TEX_NANOSUIT_BODY_DIFF_EMISS, TEX_NANOSUIT_BODY_NORM, TEX_NANOSUIT_BODY_SPEC_ROUGH);
				auto* nano1 = mesh_nano("Legs",		v3(0), quat::ident(), "nano_suit/nanosuit_legs",	MAT_IDENTITY,
						TEX_NANOSUIT_BODY_DIFF_EMISS, TEX_NANOSUIT_BODY_NORM, TEX_NANOSUIT_BODY_SPEC_ROUGH);
				auto* nano2 = mesh_nano("Neck",		v3(0), quat::ident(), "nano_suit/nanosuit_neck",	MAT_IDENTITY,
						TEX_NANOSUIT_NECK_DIFF, TEX_NANOSUIT_NECK_NORM, TEX_NANOSUIT_NECK_SPEC_ROUGH);
				auto* nano3 = mesh_nano("Helmet",	v3(0), quat::ident(), "nano_suit/nanosuit_helmet",	MAT_IDENTITY,
						TEX_NANOSUIT_HELMET_DIFF_EMISS, TEX_NANOSUIT_HELMET_NORM, TEX_NANOSUIT_HELMET_SPEC_ROUGH);
				
		auto* scn1 = scene("tree_scene",
				v3(-5.13f, -14.30f, +0.94f), quat(v3(-0.00f, -0.00f, +0.00f), +1.00f));
			
			auto* gnd1 = GROUND(5,5);
			
			auto* lgh10 = sun_lamp("Sun",
					v3(+0.06f, -2.76f, +4.53f), quat(v3(+0.31f, +0.01f, +0.04f), +0.95f),
					SHAD,
					srgb(244,217,171) * col(2000));
			auto* msh10 = mesh("terrain",
					v3(0), quat::ident(),
					"terrain.nouv",					MAT_TERRAIN);
				auto* msh11 = mesh("tree",
						v3(0), quat::ident(),
						"terrain_tree",				MAT_TREE_BARK,			TEX_TERRAIN_TREE_DIFFUSE);
				auto* msh12 = mesh("tree_cuts",
						v3(0), quat::ident(),
						"terrain_tree_cuts",		MAT_TREE_CUTS,			TEX_TERRAIN_TREE_CUTS_DIFFUSE);
				auto* msh13 = mesh("tree_blossoms",
						v3(0), quat::ident(),
						"terrain_tree_blossoms",	MAT_TREE_BLOSSOMS);
				auto* msh14 = mesh("cube",
						v3(+0.97f, +1.54f, +0.90f),
						quat(v3(+0.305f, +0.344f, +0.054f), +0.886f),
						v3(0.226f),
						"terrain_cube",				MAT_RUSTY_METAL,		TEX_TERRAIN_CUBE_DIFFUSE);
				auto* msh15 = mesh("sphere",
						v3(-1.49f, +1.45f, +0.86f),
						quat(v3(0.0f, 0.0f, -0.089f), +0.996f),
						v3(0.342f),
						"terrain_sphere",			MAT_GLASS,				TEX_TERRAIN_SPHERE_DIFFUSE);
				auto* msh16 = mesh("obelisk",
						v3(-1.49f, -1.53f, +0.83f),
						quat(v3(+0.005f, +0.01f, -0.089f), +0.996f),
						"terrain_obelisk",			MAT_OBELISK,			TEX_TERRAIN_OBELISK_DIFFUSE);
				auto* msh17 = mesh("teapot",
						v3(+1.49f, -1.445f, +0.284f),
						quat(v3(-0.025f, -0.055f, -0.561f), +0.826f),
						"utah_teapot.nouv",			MAT_SHOW_COPPER);
			
		auto* scn2 = scene("ugly_scene",
				v3(+3.70f, -10.70f, +0.94f), quat(v3(-0.00f, -0.00f, +0.00f), +1.00f));
			
			auto* gnd2 = GROUND(3,3);
			
			auto* lgh20 = sun_lamp("Sun",
					v3(-1.83f, +1.37f, +2.05f), quat(v3(+0.21f, -0.45f, -0.78f), +0.37f),
					SHAD, srgb(244,217,171) * col(2000));
			auto* msh20 = mesh("ugly",
					v3(0), quat(v3(-0.00f, +0.00f, -1.00f), +0.00f),
					"ugly", MAT_SHINY_PLATINUM,
					ROUGH(TEX_MAT_ROUGHNESS));
			
		auto* scn3 = scene("structure_scene",
				v3(0), quat::ident());
			
			auto* lgh30 = sun_lamp("Sun",		v3(-2.84f, +4.70f, +4.90f), quat(v3(+0.15f, -0.12f, -0.62f), +0.76f),
					SHAD, srgb(244,217,171) * col(2000));
			
			auto* msh3z = mesh("ground",		v3(0), quat::ident(),
					"scene_ground0",		MAT_GRASS,						TEX_GRASS);
			auto* msh30 = mesh("ring",			v3(0), quat::ident(),
					"structure_ring.nouv",	MAT_GRIPPED_METAL);
			auto* msh31 = mesh("walls",			v3(0), quat::ident(),
					"structure_walls",		MAT_BLOTCHY_METAL,				TEX_STRUCTURE_WALLS);
			auto* msh32 = mesh("ground",		v3(0), quat::ident(),
					"structure_ground",		MAT_DIRT,						TEX_STRUCTURE_GROUND);
			auto* msh33 = mesh("block 1",		v3(+1.52488f, +0.41832f, -3.31113f), quat(v3(-0.012f, +0.009f, -0.136f), +0.991f),
					"structure_block1",		MAT_GRIPPED_METAL,				TEX_METAL_GRIPPED_DIFFUSE);
			auto* msh34 = mesh("block 2",		v3(+3.05563f, +5.89111f, +0.53238f), quat(v3(-0.038f, -0.001f, +0.076f), +0.996f),
					"structure_block2",		MAT_RUSTY_METAL,				TEX_METAL_RUSTY_02);
			auto* msh35 = mesh("block 3",		v3(-2.84165f, +4.51917f, -2.67442f), quat(v3(-0.059f, -0.002f, +0.056f), +0.997f),
					"structure_block3",		MAT_RUSTY_METAL,				TEX_METAL_RUSTY_02);
			auto* msh36 = mesh("block 4",		v3(+0.69161f, +1.3302f, -2.57026f), quat(v3(-0.013f, +0.009f, -0.253f), +0.967f),
					"structure_block4",		MAT_MARBLE,						TEX_MARBLE);
					
			auto* msh37 = group("beam",			v3(-3.4297f, +1.47318f, -1.26951f), quat(v3(-0.088f, +0.017f, +0.996f), +0.0008585f));
				
				auto* msh37_0 = mesh("beam",	v3(0), quat::ident(),
						"structure_beam",	MAT_WOODEN_BEAM,				TEX_WOODEN_BEAM);
				auto* msh37_1 = mesh("cuts",	v3(0), quat::ident(),
						"structure_beam_cuts",	MAT_WOODEN_BEAM_CUTS,	TEX_TERRAIN_TREE_CUTS_DIFFUSE);
			
		auto* scn4 = scene("normals",			v3(-8.62f, +1.19f, +0.00f), quat(v3(-0.00f, -0.00f, +0.00f), +1.00f));
			
			auto* gnd4 = GROUND(3,3);
			
			auto* lgh40 = sun_lamp("Sun",		v3(+1.88f, +2.46f, +3.35f), quat(v3(+0.28f, +0.48f, +0.72f), +0.42f),
					SHAD, srgb(244,217,171) * col(2000));
			auto* msh40 = mesh("brick_wall 1",	v3(+0.00f, +1.15f, +1.01f), quat(v3(+0.61f, +0.36f, +0.36f), +0.61f),
					"_unit_plane",			MAT_GRASS,		TEX_TEX_DIF_BRICK_00,	TEX_TEX_NRM_BRICK_00); // met Supposed to be rough stone
			auto* msh41 = mesh("brick_wall 2",	v3(+1.09f, +2.94f, +0.9f), quat(v3(+0.62f, +0.33f, +0.33f), +0.62f),
					"_unit_plane",			MAT_GRASS,		TEX_TEX_DIF_BRICK_01,	TEX_TEX_NRM_BRICK_01); // met Supposed to be rough stone
			auto* msh42 = mesh("weird plane",	v3(-0.73f, -1.07f, +0.24f), quat(v3(+0.06f, +0.08f, +0.80f), +0.59f),
					"norm_test/test_00",	MAT_PLASTIC,	TEX_IDENT,				TEX_NORM_TEST_00);
			auto* msh43 = mesh("david",			v3(+1.54f, +0.80f, -0.01f), quat(v3(-0.00f, +0.00f, -0.76f), +0.65f),
					"photogr/david",		MAT_ROUGH_MARBLE,	TEX_PG_DAVID_ALBEDO,	TEX_PG_DAVID_NORMAL);
			auto* msh44 = mesh("stool",			v3(+1.60f, +1.5f, +0.4f), quat(v3(-0.00f, +0.00f, -0.76f), +0.65f),
					"photogr/stool",		MAT_WOODEN_BEAM,	TEX_PG_STOOL_ALB,		TEX_PG_STOOL_NRM_256);
			auto* msh45 = mesh("david3",		v3(+1.61f, +1.54f, +0.4f), quat(v3(-0.00f, +0.00f, -0.76f), +0.65f),
					"photogr/david3",		MAT_GLASS,	TEX_PG_DAVID3_ALB,		TEX_PG_DAVID3_NRM);
			
		auto* scn5 = scene("PBR showcase",
				v3(+9.67f, +0.16f, +0.00f), quat(v3(-0.00f, -0.00f, +0.71f), +0.71f));
			
			auto* gnd5 = GROUND(17,7);
			
			auto* lgh50 = sun_lamp("Sun",		v3(-4.91f, +2.46f, +3.35f), quat(v3(+0.05f, -0.22f, -0.95f), +0.21f),
					SHAD, srgb(244,217,171) * col(2000));
			auto* msh50 = material_showcase_grid("ico_sphere",	v3(-4.84f, -1.52f, +0.00f), quat::ident(), v3(0.3f),
					"_ico_sphere.nouv", v2(2.5f));
			auto* msh51 = material_showcase_grid("bunny",		v3(+0.38f, -1.69f, +0.00f), quat::ident(), v3(2.0f),
					"stanford/bunny.nouv", v2(0.375f));
			auto* msh52 = material_showcase_grid("buddha",		v3(+1.25f, -4.38f, +0.00f), quat::ident(), v3(1),
					"stanford/buddha.nouv", v2(0.85f));
			auto* msh53 = material_showcase_grid("dragon",		v3(-3.41f, +1.48f, +0.00f), quat::ident(), v3(1),
					"stanford/dragon.nouv", v2(0.65f));
			auto* msh54 = material_showcase_grid("teapot",		v3(+2.09f, +1.42f, +0.00f), quat::ident(), v3(2.5f),
					"utah_teapot.nouv", v2(0.3f));
			
			auto* grp55 = group("materials",	v3(-13.89f, -0.74f, +1.00f), quat(v3(+0.00f, -0.00f, -0.33f), +0.94f));
				
				v3 offs = v3(0.65f, 0,0);
				
				auto* msh550 = mesh("plastic",	v3(0)*offs, quat::ident(), v3(0.3f),	"_ico_sphere.nouv", MAT_SHOW_PLASTIC);
				auto* msh551 = mesh("glass",	v3(1)*offs, quat::ident(), v3(0.3f),	"_ico_sphere.nouv", MAT_SHOW_GLASS);
				auto* msh552 = mesh("plasic_h",	v3(2)*offs, quat::ident(), v3(0.3f),	"_ico_sphere.nouv", MAT_SHOW_PLASIC_H);
				auto* msh553 = mesh("ruby",		v3(3)*offs, quat::ident(), v3(0.3f),	"_ico_sphere.nouv", MAT_SHOW_RUBY);
				auto* msh554 = mesh("diamond",	v3(4)*offs, quat::ident(), v3(0.3f),	"_ico_sphere.nouv", MAT_SHOW_DIAMOND);
				auto* msh555 = mesh("iron",		v3(5)*offs, quat::ident(), v3(0.3f),	"_ico_sphere.nouv", MAT_SHOW_IRON);
				auto* msh556 = mesh("copper",	v3(6)*offs, quat::ident(), v3(0.3f),	"_ico_sphere.nouv", MAT_SHOW_COPPER);
				auto* msh557 = mesh("gold",		v3(7)*offs, quat::ident(), v3(0.3f),	"_ico_sphere.nouv", MAT_SHOW_GOLD);
				auto* msh558 = mesh("alu",		v3(8)*offs, quat::ident(), v3(0.3f),	"_ico_sphere.nouv", MAT_SHOW_ALU);
				auto* msh559 = mesh("silver",	v3(9)*offs, quat::ident(), v3(0.3f),	"_ico_sphere.nouv", MAT_SHOW_SILVER);
				
			auto* msh56 = mesh("cerberus",	v3(-7.41f, -1.04f, +0.70f), quat(v3(-0.07f, +0.01f, -0.13f), +0.99f), v3(1),
					"cerberus/cerberus",			MAT_IDENTITY,
					TEX_CERBERUS_ALBEDO, TEX_CERBERUS_NORMAL, TEX_CERBERUS_ROUGHNESS, TEX_CERBERUS_METALLIC);
			
			auto* lgh57 = light_bulb("Grey dim",	v3(-6.28f, +0.24f, +0.83f), ON, srgb(225.0f,228,230) * col(75));
			auto* lgh58 = light_bulb("Blue",		v3(-7.10f, -3.60f, +1.46f), ON, srgb(29,54,252) * col(450));
			auto* lgh59 = light_bulb("Red bright",	v3(-12.38f, -4.31f, +2.65f), ON, srgb(237,7,51) * col(750));
			auto* lgh5a = light_bulb("Sunlike",	v3(-0.30f, -2.18f, +3.60f), ON, srgb(244,217,171) * col(1200));
			
			auto* grp5b = group("nanosuit",		v3(-8.88f, -0.65f, +0.00f), quat(v3(-0.00f, +0.00f, -0.24f), +0.97f));
				
				auto* nano50 = mesh_nano("Torso",	v3(0), quat::ident(), "nano_suit/nanosuit_torso",	MAT_IDENTITY,
						TEX_NANOSUIT_BODY_DIFF_EMISS, TEX_NANOSUIT_BODY_NORM, TEX_NANOSUIT_BODY_SPEC_ROUGH);
				auto* nano51 = mesh_nano("Legs",	v3(0), quat::ident(), "nano_suit/nanosuit_legs",	MAT_IDENTITY,
						TEX_NANOSUIT_BODY_DIFF_EMISS, TEX_NANOSUIT_BODY_NORM, TEX_NANOSUIT_BODY_SPEC_ROUGH);
				auto* nano52 = mesh_nano("Neck",	v3(0), quat::ident(), "nano_suit/nanosuit_neck",	MAT_IDENTITY,
						TEX_NANOSUIT_NECK_DIFF, TEX_NANOSUIT_NECK_NORM, TEX_NANOSUIT_NECK_SPEC_ROUGH);
				auto* nano53 = mesh_nano("Helmet",	v3(0), quat::ident(), "nano_suit/nanosuit_helmet",	MAT_IDENTITY,
						TEX_NANOSUIT_HELMET_DIFF_EMISS, TEX_NANOSUIT_HELMET_NORM, TEX_NANOSUIT_HELMET_SPEC_ROUGH);
				
			
		
		tree(&root,
			scene_tree(scn,
				gnd0,
				lgh0,
				tree(msh0,
					lgh1,
					lgh2,
					lgh3
				),
				tree(msh1,
					lgh4,
					lgh5
				),
				tree(nano,
					nano0,
					nano1,
					nano2,
					nano3
				)
			),
			scene_tree(scn1,
				gnd1,
				lgh10,
				tree(msh10,
					msh11,
					msh12,
					msh13,
					msh14,
					msh15,
					msh16,
					msh17
				)
			),
			scene_tree(scn2,
				gnd2,
				lgh20,
				msh20
			),
			scene_tree(scn3,
				lgh30,
				msh3z,
				msh30,
				msh31,
				msh32,
				msh33,
				msh34,
				msh35,
				msh36,
				tree(msh37,
					msh37_0,
					msh37_1
				)
			),
			scene_tree(scn4,
				gnd4,
				lgh40,
				msh40,
				msh41,
				msh42,
				msh43,
				msh44,
				msh45
			),
			scene_tree(scn5,
				gnd5,
				lgh50,
				msh50,
				msh51,
				msh52,
				msh53,
				msh54,
				tree(grp55,
					msh550,
					msh551,
					msh552,
					msh553,
					msh554,
					msh555,
					msh556,
					msh557,
					msh558,
					msh559
				),
				msh56,
				lgh57,
				lgh58,
				lgh59,
				lgh5a,
				tree(grp5b,
					nano50,
					nano51,
					nano52,
					nano53
				)
			)
		);
		
		//test_enumerate();
		//
		//print(">>> Meshes:\n");
		//scenes[0]->meshes.templ_forall( [] (Mesh* e) { print("  %\n", e->name); } );
		//
		//print(">>> Lights:\n");
		//scenes[0]->lights.templ_forall( [] (Light* e) { print("  %\n", e->name); } );
		
	}
	
	entity_flag _calc_mesh_aabb (Entity* e, hm mp parent_to_world) {
		
		entity_flag ret = (entity_flag)0;
		
		AABB aabb_mesh;
		
		switch (e->tag) {
			case ET_MESH:
			case ET_MESH_NANOSUIT: {
				auto* m = (eMesh_Base*)e;
				aabb_mesh = m->mesh->aabb;
				ret = EF_HAS_MESHES;
			} break;
			
			case ET_MATERIAL_SHOWCASE_GRID: {
				aabb_mesh = calc_material_showcase_grid_aabb((Material_Showcase_Grid*)e);
				ret = EF_HAS_MESHES;
			} break;
			
			case ET_LIGHT: {
				auto* l = (Light*)e;
				aabb_mesh = l->mesh->aabb;
				ret = EF_HAS_MESHES;
			} break;
			
			case ET_GROUP: {
			case ET_SCENE:
				aabb_mesh = AABB::inf();
			} break;
				
			default: assert(false);
		}
		
		hm to_parent = me_to_parent(e);
		hm to_world = parent_to_world * to_parent;
		
		if (ret) {
			auto aabb_mesh_corners = aabb_mesh.box_corners();
			
			e->aabb_mesh_paren = AABB::from_obb(to_parent * aabb_mesh_corners);
			e->aabb_mesh_world = AABB::from_obb(to_world * aabb_mesh_corners);
			
			//dbg_lines.push_box_world(parent_to_world * to_parent, aabb_mesh.box_edges(), v3(0.2f));
		} else {
			e->aabb_mesh_paren = AABB::inf();
			e->aabb_mesh_world = AABB::inf();
		}
		
		auto* c = e->children;
		while (c) {
			auto res = _calc_mesh_aabb(c, to_world);
			if (res) {
				ret |= res;
				
				e->aabb_mesh_paren.minmax( to_parent * c->aabb_mesh_paren.box_corners() );
				e->aabb_mesh_world.minmax( c->aabb_mesh_world );
			}
			
			c = c->next;
		}
		
		if (ret) {
		//	dbg_lines.push_box_world(parent_to_world, e->aabb_mesh_paren.box_edges(), v3(1.0f));
		//	dbg_lines.push_box_world(hm::ident(), e->aabb_mesh_world.box_edges(), v3(0.25f));
		}
		
		e->eflags &= ~EF_HAS_MESHES;
		e->eflags |= ret;
		return ret;
	}
	void calc_mesh_aabb () {
		PROFILE_SCOPED(THR_ENGINE, "entities_calc_mesh_aabb");
		
		auto* e = root.children;
		while (e) {
			_calc_mesh_aabb(e, hm::ident());
			
			e = e->next;
		}
	}
	
};

AABB calc_shadow_cast_aabb (Entity const* e, hm mp parent_to_light) {
	
	AABB aabb_me;
	
	switch (e->tag) {
		case ET_MESH:
		case ET_MESH_NANOSUIT: {
			auto* m = (eMesh_Base*)e;
			aabb_me = m->mesh->aabb;
		} break;
		case ET_MATERIAL_SHOWCASE_GRID: {
			aabb_me = calc_material_showcase_grid_aabb((Material_Showcase_Grid*)e);
		} break;
		
		case ET_GROUP:
		case ET_LIGHT:
		case ET_SCENE: {
			aabb_me = AABB::inf();
		} break;
			
		default: assert(false);
	}
	
	hm me_to_light = parent_to_light * me_to_parent(e);
	
	AABB aabb_light;
	if (!aabb_me.is_inf()) {
		aabb_light = AABB::from_obb(me_to_light * aabb_me.box_corners());
	} else {
		aabb_light = aabb_me; // inf
	}
	
	auto* c = e->children;
	while (c) {
		auto child_aabb_light = calc_shadow_cast_aabb(c, me_to_light);
		if (!child_aabb_light.is_inf()) {
			aabb_light.minmax( child_aabb_light ); // child_aabb_light might be inf
		}
		
		c = c->next;
	}
	
	return aabb_light;
}
