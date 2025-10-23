#define TINYOBJLOADER_IMPLEMENTATION

#include "model.h"

#include "utils/error_handler.h"

#include <linalg.h>


using namespace linalg::aliases;
using namespace cg::world;

cg::world::model::model() {}

cg::world::model::~model() {}

void cg::world::model::load_obj(const std::filesystem::path& model_path)
{
	// TODO Lab: 1.03 Using `tinyobjloader` implement `load_obj`, `allocate_buffers`, `compute_normal`, `fill_vertex_data`, and `fill_buffers` methods of `cg::world::model` class
	// Парсинг OBJ/MTL
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	std::filesystem::path base = model_path.parent_path();
	bool ok = tinyobj::LoadObj(
		&attrib, &shapes, &materials,
		&warn, &err,
		model_path.string().c_str(),
		base.string().c_str(),
		/*triangulate*/ true,
		/*default_vcols_fallback*/ false
	);

	// if (!warn.empty()) {
	// 	// не критично, просто информируем
	// 	std::cerr << "tinyobj warning: " << warn << std::endl;
	// }
	if (!ok || !err.empty()) {
		THROW_ERROR(std::string("tinyobj error: ") + err);
	}

	// Выделение буферов на шейп
	allocate_buffers(shapes);

	// Заполнение VB/IB и списка текстур
	fill_buffers(shapes, attrib, materials, base);
}

void model::allocate_buffers(const std::vector<tinyobj::shape_t>& shapes)
{
	// TODO Lab: 1.03 Using `tinyobjloader` implement `load_obj`, `allocate_buffers`, `compute_normal`, `fill_vertex_data`, and `fill_buffers` methods of `cg::world::model` class
	vertex_buffers.clear();
	index_buffers.clear();
	textures.clear();
	vertex_buffers.reserve(shapes.size());
	index_buffers.reserve(shapes.size());
	textures.reserve(shapes.size());

	for (const auto& s : shapes)
	{
		// Оценить верхнюю границу количества вершин по количеству индексов
		// (будет один к одному после развёртки индексов).
		size_t index_count = s.mesh.indices.size();
		// Создаём ресурсы: VB размером = index_count вершин, IB = index_count индексов
		auto vb = std::make_shared<cg::resource<cg::vertex>>(index_count);
		auto ib = std::make_shared<cg::resource<unsigned int>>(index_count);

		vertex_buffers.push_back(vb);
		index_buffers.push_back(ib);

		// Плейсхолдер под путь текстуры для шейпа (если будет)
		textures.emplace_back(std::filesystem::path{});
	}
}

float3 cg::world::model::compute_normal(const tinyobj::attrib_t& attrib, const tinyobj::mesh_t& mesh, size_t index_offset)
{
	// TODO Lab: 1.03 Using `tinyobjloader` implement `load_obj`, `allocate_buffers`, `compute_normal`, `fill_vertex_data`, and `fill_buffers` methods of `cg::world::model` class
	// Вычисление геометрической нормали треугольника по 3 вершинам (позициям)
	// index_offset указывает на начало треугольника в mesh.indices (индекс a).
	const tinyobj::index_t ia = mesh.indices[index_offset + 0];
	const tinyobj::index_t ib = mesh.indices[index_offset + 1];
	const tinyobj::index_t ic = mesh.indices[index_offset + 2];

	auto fetch_pos = [&](const tinyobj::index_t& idx)->float3 {
		size_t vi = static_cast<size_t>(idx.vertex_index);
		float x = attrib.vertices[3 * vi + 0];
		float y = attrib.vertices[3 * vi + 1];
		float z = attrib.vertices[3 * vi + 2];
		return float3{ x, y, z };
	};

	float3 pa = fetch_pos(ia);
	float3 pb = fetch_pos(ib);
	float3 pc = fetch_pos(ic);

	float3 e1 = pb - pa;
	float3 e2 = pc - pa;

	// cross и normalize из linalg
	float3 n = normalize(cross(e1, e2));
	// На случай вырожденного треугольника
	if (!std::isfinite(n.x) || !std::isfinite(n.y) || !std::isfinite(n.z)) {
		n = float3{ 0.f, 0.f, 1.f };
	}
	return n;
}

void model::fill_vertex_data(cg::vertex& vertex, const tinyobj::attrib_t& attrib, const tinyobj::index_t idx, const float3 computed_normal, const tinyobj::material_t material)
{
	// TODO Lab: 1.03 Using `tinyobjloader` implement `load_obj`, `allocate_buffers`, `compute_normal`, `fill_vertex_data`, and `fill_buffers` methods of `cg::world::model` class
	// Позиция
	{
		size_t vi = static_cast<size_t>(idx.vertex_index);
		vertex.position = float3{
			attrib.vertices[3 * vi + 0],
			attrib.vertices[3 * vi + 1],
			attrib.vertices[3 * vi + 2]
		};
	}

	// Нормаль: берем из атрибутов, если есть; иначе — скомпьютенную
	if (idx.normal_index >= 0) {
		size_t ni = static_cast<size_t>(idx.normal_index);
		vertex.normal = float3{
			attrib.normals[3 * ni + 0],
			attrib.normals[3 * ni + 1],
			attrib.normals[3 * ni + 2]
		};
	} else {
		vertex.normal = computed_normal;
	}

	// UV: если есть
	if (idx.texcoord_index >= 0) {
		size_t ti = static_cast<size_t>(idx.texcoord_index);
		vertex.texcoord = float2{
			attrib.texcoords[2 * ti + 0],
			attrib.texcoords[2 * ti + 1]
		};
	} else {
		vertex.texcoord = float2{ 0.f, 0.f };
	}
}


void model::fill_buffers(const std::vector<tinyobj::shape_t>& shapes, const tinyobj::attrib_t& attrib, const std::vector<tinyobj::material_t>& materials, const std::filesystem::path& base_folder)
{
	// TODO Lab: 1.03 Using `tinyobjloader` implement `load_obj`, `allocate_buffers`, `compute_normal`, `fill_vertex_data`, and `fill_buffers` methods of `cg::world::model` class
	for (size_t s = 0; s < shapes.size(); ++s)
	{
		const auto& shape = shapes[s];
		const auto& mesh = shape.mesh;

		// Привязанные ресурсы для шейпа
		auto vb = vertex_buffers[s];
		auto ib = index_buffers[s];

		// Индексная запись линейная: на каждый tinyobj индекс — уникальная вершина в нашем VB
		size_t write = 0;
		for (size_t f = 0; f < mesh.num_face_vertices.size(); ++f)
		{
			const size_t fv = static_cast<size_t>(mesh.num_face_vertices[f]);
			// Ожидаем triangulate=true => fv == 3
			if (fv != 3) {
				// пропускаем не‑треугольные фейсы на всякий случай
				continue;
			}
			size_t index_offset = 0;
			for (size_t k = 0; k < f; ++k) {
			index_offset += static_cast<size_t>(mesh.num_face_vertices[k]);
			}
			// Вычислим нормаль треугольника, если нормалей нет
			float3 tri_normal = compute_normal(attrib, mesh, index_offset);

			// Материал для этой грани, если есть
			tinyobj::material_t mtl{};
			int mtl_id = mesh.material_ids.size() > f ? mesh.material_ids[f] : -1;
			if (mtl_id >= 0 && static_cast<size_t>(mtl_id) < materials.size()) {
				mtl = materials[static_cast<size_t>(mtl_id)];
			}

			// Три вершины
			for (size_t v = 0; v < fv; ++v) {
				const tinyobj::index_t idx = mesh.indices[index_offset + v];

				// Записываем вершину
				cg::vertex vert{};
				fill_vertex_data(vert, attrib, idx, tri_normal, mtl);
				vb->item(write) = vert;

				// Индекс указывает на только что записанную вершину
				ib->item(write) = static_cast<unsigned int>(write);

				++write;
			}
		}

		// Текстуры на шейп: если материал назначен и есть diffuse_texname, сохраним относительный путь
		// Берём первый валидный материал у шейпа.
		std::filesystem::path tex_path{};
		for (int mid : mesh.material_ids) {
			if (mid >= 0 && static_cast<size_t>(mid) < materials.size()) {
				const auto& mtl = materials[static_cast<size_t>(mid)];
				if (!mtl.diffuse_texname.empty()) {
					tex_path = (base_folder / mtl.diffuse_texname).lexically_normal();
					break;
				}
			}
		}
		textures[s] = tex_path;
	} //for
}


const std::vector<std::shared_ptr<cg::resource<cg::vertex>>>&
cg::world::model::get_vertex_buffers() const
{
	return vertex_buffers;
}

const std::vector<std::shared_ptr<cg::resource<unsigned int>>>&
cg::world::model::get_index_buffers() const
{
	return index_buffers;
}

const std::vector<std::filesystem::path>& cg::world::model::get_per_shape_texture_files() const
{
	return textures;
}


const float4x4 cg::world::model::get_world_matrix() const
{
	return float4x4{
			{1, 0, 0, 0},
			{0, 1, 0, 0},
			{0, 0, 1, 0},
			{0, 0, 0, 1}};
}
