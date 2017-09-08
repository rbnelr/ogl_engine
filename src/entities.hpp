
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
		EF_HAS_AABB=	0b01,
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
	
	struct Transform {
		// Forward transform is always scale first, orientation second and position last
		
		v3				pos;
		quat			ori;
		v3				scale;
		
		hm				forw;
		hm				inv;
		
		DECLM static constexpr Transform qnan () {
			return { v3(QNAN), quat(v3(QNAN), QNAN), v3(QNAN), hm::all(QNAN), hm::all(QNAN) };
		}
		DECLM void calc_transform () {
			forw =	translate_h(pos) * hm::ident().m3(conv_to_m3(ori)) * scale_h(scale);
			inv =	scale_h(v3(1) / scale) * hm::ident().m3(conv_to_m3(inverse(ori))) * translate_h(-pos);
		}
	};
	
	struct Entity {
		lstr			name;
		u32				depth; // depth in hierarchy
		
		entity_flag		eflags; // might go away, only used to identify entities with and without AABB (is it selectable)
		entity_tag		tag;
		
		eid				next;
		eid				parent;
		eid				children;
		
		Transform		to_paren;
		AABB			mesh_aabb_paren; // only valid if (eflags & EF_HAS_AABB)
		
		Transform		to_world;
		AABB			mesh_aabb_world; // only valid if (eflags & EF_HAS_AABB)
		
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
								Transform::qnan(), AABB::qnan(),
								Transform::qnan(), AABB::qnan() };
			cmemset(&ret.base +1, DBG_MEM_UNINITIALIZED_BYTE, sizeof(Entity_union) -sizeof(Entity));
			return ret;
		}
		
		DECLM void calc_transforms_and_aabb (Entity_union const* paren) {
			base.to_paren.calc_transform();
			
			//base.to_world.pos =		(base.to_paren.forw * hv(base.to_paren.pos)).v3();
			//base.to_world.ori =		(base.to_paren.forw * hv(base.to_paren.pos)).v3();
			//base.to_world.scale;
			
			base.to_world.forw =	base.to_paren.forw;
			base.to_world.inv =		base.to_paren.inv;
			if (paren) {
				base.to_world.forw =	paren->base.to_world.forw * base.to_world.forw;
				base.to_world.inv *=	paren->base.to_world.inv;
			}
			
		}
		
		#if 0
		DECLM void calc_transforms_and_aabb (Entity_union const* paren) {
			base.to_paren.calc_transform();
			base.aabb_mesh_paren = to_space(base.to_paren.forw) mesh->aabb
			
		}
		
		template <bool SHADOW_CAST_AABB>
		DECLM void calc_aabb
		
		entity_flag _calc_mesh_aabb (Entity_union* e, hm mp parent_to_world) {
			
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
		#endif
	};
};
using namespace entities_n;

struct Entities {
	
	dynarr<Entity_union> storage; // all entities in one array as unions
	
	DECLM void init () {
		storage = storage.alloc(256);
		
		storage.push() = Entity_union::null();
	}
	
	DECLM Entity* get (eid id) {
		assert(id > 0);
		return &storage[id].base;
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
		return ret;
	}
	DECLM void pop () {
		storage.pop();
	}
	
	DECLM void _calc_entities_transforms_and_aabb_recurse (Entity_union* e, Entity_union const* paren, u32 depth) {
		for (;;) {
			e->base.depth = depth;
			e->calc_transforms_and_aabb(paren);
			if (e->base.children) _calc_entities_transforms_and_aabb_recurse(&storage[e->base.children], e, depth +1);
			if (!e->base.next) break;
			e = &storage[e->base.next];
		}
	}
	DECLM void calc_entities_transforms_and_aabb () {
		if (storage.len > 1) _calc_entities_transforms_and_aabb_recurse(&storage[1], nullptr, 0);
	}
	
};
