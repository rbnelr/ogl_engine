
namespace entities_n {
	
	typedef u32 eid; // entity id is simply index in all-entities array, entities are never deleted
	
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
		EF_CASTS_SHADOW=	0b01,
	}; DEFINE_ENUM_FLAG_OPS(entity_flag, u32)
	
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
		u32				depth; // depth in hierarchy
		
		entity_flag		eflags;
		entity_tag		tag;
		
		eid				next;
		eid				parent;
		eid				children;
		
		// in parent space
		v3				pos;
		quat			ori;
		v3				scale;
		
		hm				to_paren;
		hm				from_paren;
		
		hm				to_world;
		hm				from_world;
		
		AABB			select_aabb_paren;
		AABB			shadow_aabb_paren; // only valid if (eflags & EF_CASTS_SHADOW)
		
		DECLM void calc_paren_transform () {
			to_paren =		translate_h(pos) * hm::ident().m3(conv_to_m3(ori)) * scale_h(scale);
			from_paren =	scale_h(v3(1) / scale) * hm::ident().m3(conv_to_m3(inverse(ori))) * translate_h(-pos);
		}
		DECLM void calc_transforms (Entity const* paren) {
			calc_paren_transform();
			
			to_world =		to_paren;
			from_world =	from_paren;
			if (paren) {
				to_world =		paren->to_world * to_world;
				from_world *=	paren->from_world;
			}
			
		}
		
		DECLM void aabb_set_inf () {
			select_aabb_paren = AABB::inf();
			shadow_aabb_paren = AABB::inf();
		}
		
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
		
		DECLM AABB calc_aabb () {
			
			assert(material_showcase_grid_steps.x >= 1 && material_showcase_grid_steps.y >= 1);
			
			AABB aabb_mesh = mesh->aabb;
			
			auto mat = translate_h(v3( material_showcase_grid_mat * (cast_v2<v2>(material_showcase_grid_steps -v2u32(1)) * grid_offs), 0.0f ));
			auto aabb_outter = mat * aabb_mesh.box_corners();
			
			return aabb_mesh.minmax(aabb_outter);
		}
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
	
	union Entity_union {
		Entity					base;
		eMesh_Base				eMesh_Base;
		eMesh					eMesh;
		eMesh_Nanosuit			eMesh_Nanosuit;
		Material_Showcase_Grid	Material_Showcase_Grid;
		Group					Group;
		Light					Light;
		Scene					Scene;
		
		DECLM FORCEINLINE Entity_union () {}
		DECLM static FORCEINLINE Entity_union null () {
			Entity_union ret;
			ret.base = Entity{	"<null>", (u32)-1,
								(entity_flag)-1, (entity_tag)-1,
								0,0,0,
								v3(QNAN), quat(v3(QNAN), QNAN), v3(QNAN),
								hm::ident(), hm::ident(), hm::ident(), hm::ident(), // to allow conditionless getting of parent to world matrix
								AABB::qnan(), AABB::qnan() };
			cmemset(&ret.base +1, DBG_MEM_UNINITIALIZED_BYTE, sizeof(Entity_union) -sizeof(Entity));
			return ret;
		}
		
		DECLM void aabb_calc (Entity* paren) {
			
			// aabb were minmax'ed with our children aabbs, if we don't have children they are still inf
			
			AABB aabb_mesh;
			
			switch (base.tag) {
				case ET_MESH:
				case ET_MESH_NANOSUIT: {
					aabb_mesh = eMesh_Base.mesh->aabb;
					
				} break;
				
				case ET_MATERIAL_SHOWCASE_GRID: {
					aabb_mesh = Material_Showcase_Grid.calc_aabb();
				} break;
				
				case ET_LIGHT: {
					aabb_mesh = Light.mesh->aabb;
				} break;
				
				case ET_GROUP:
				case ET_SCENE: {
					// keep potential inf
				} break;
					
				default: assert(false);
			}
			
			switch (base.tag) {
				case ET_MESH:
				case ET_MESH_NANOSUIT:
				case ET_MATERIAL_SHOWCASE_GRID:
				case ET_LIGHT: {
					auto aabb_paren = AABB::from_obb(base.to_paren * aabb_mesh.box_corners());
					
					base.select_aabb_paren.minmax( aabb_paren );
					if (base.tag != ET_LIGHT) {
						base.shadow_aabb_paren.minmax( aabb_paren );
					}
				} break;
				
				case ET_GROUP:
				case ET_SCENE: {
					
				} break;
					
				default: assert(false);
			}
			
			f32 dummy_aabb_radius = 0.15f;
			AABB dummy = {	-dummy_aabb_radius,+dummy_aabb_radius,
							-dummy_aabb_radius,+dummy_aabb_radius,
							-dummy_aabb_radius,+dummy_aabb_radius };
			
			if (base.select_aabb_paren.is_inf()) {
				base.select_aabb_paren = dummy +base.pos;
			}
			if (paren) paren->select_aabb_paren.minmax( paren->to_paren * base.select_aabb_paren.box_corners() );
			
			base.eflags &= ~EF_CASTS_SHADOW;
			if (base.shadow_aabb_paren.is_inf()) {
				// do not attribute to parent aabb
			} else {
				base.eflags |= EF_CASTS_SHADOW;
				if (paren) paren->shadow_aabb_paren.minmax( paren->to_paren * base.shadow_aabb_paren.box_corners() );
			}
			
		}
		
	};
};
using namespace entities_n;

struct Entities {
	
	dynarr<Entity_union> storage; // all entities in one array as unions
	
	eid		first;
	
	DECLM void init () {
		storage = storage.alloc(256);
		
		storage.push() = Entity_union::null();
	}
	DECLM void _clear () { // for testing
		storage.clear(256);
		
		storage.push() = Entity_union::null();
		first = 0;
	}
	
	DECLM Entity* get_null_valid (eid id) {
		return &storage[id].base;
	}
	DECLM Entity* get (eid id) {
		assert(id > 0);
		return id != 0 ? get_null_valid(id) : nullptr;
	}
	DECLM eid search (lstr cr name) {
		for (eid i=1; i<storage.len; ++i) {
			if (str::comp(storage[i].base.name, name)) return i;
		}
		return 0;
	}
	DECLM Entity* search_and_get (lstr cr name) {
		return get(search(name));
	}
	
	DECLM eid push () {
		auto ret = storage.len;
		storage.push();
		if (ret == 1) first = ret;
		return ret;
	}
	DECLM void pop () {
		storage.pop();
	}
	
	DECLM void _calc_entities_transforms_and_aabb_recurse (eid id, Entity* paren, u32 depth) {
		do {
			auto* e = &storage[id];
			
			e->base.depth = depth;
			e->base.calc_transforms(paren);
			e->base.aabb_set_inf();
			if (e->base.children) _calc_entities_transforms_and_aabb_recurse(e->base.children, &e->base, depth +1);
			e->aabb_calc(paren);
			
			id = e->base.next;
		} while (id);
	}
	
	DECLM void calc_entities_and_aabb_transforms () {
		if (first) _calc_entities_transforms_and_aabb_recurse(first, nullptr, 0);
	}
	
};
