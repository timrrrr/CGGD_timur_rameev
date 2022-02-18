#define TINYOBJLOADER_IMPLEMENTATION

#include "model.h"

#include "utils/error_handler.h"

#include <linalg.h>


using namespace linalg::aliases;
using namespace cg::world;

cg::world::model::model() {}

cg::world::model::~model() {}

void cg::world::model::load_obj(const std::filesystem::path& model_path) {
	tinyobj::ObjReaderConfig reader_config;

	// Path to material files
	reader_config.mtl_search_path = model_path.parent_path().string();
	reader_config.triangulate = true;

	tinyobj::ObjReader reader;

	if (!reader.ParseFromFile(model_path.string(), reader_config)) {
		if (!reader.Error().empty()) {
			std::cerr << "TinyObjReader: " << reader.Error();
		}
		exit(1);
	}
	if (!reader.Warning().empty()) {
		std::cout << "TinyObjReader: " << reader.Warning();
	}

	auto& attrib = reader.GetAttrib();
	auto& shapes = reader.GetShapes();
	auto& materials = reader.GetMaterials();

	// first loop
	// Loop over shapes
	for (size_t s = 0; s < shapes.size(); s++) {
		// Loop over faces(polygon)
		size_t index_offset = 0;
		unsigned int vertex_buffer_size = 0;
		unsigned int index_buffer_size = 0;
		std::map<std::tuple<int, int, int>, unsigned int> index_map;
		const auto& mesh = shapes[s].mesh;
		for (size_t f = 0; f < mesh.num_face_vertices.size(); f++) {
			int fv = mesh.num_face_vertices[f];
			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++) {
				// access to vertex
				tinyobj::index_t idx = mesh.indices[index_offset + v];
				auto idx_tuple = std::make_tuple(
						idx.vertex_index, idx.normal_index, idx.texcoord_index);
				if (index_map.count(idx_tuple) == 0) {
					index_map[idx_tuple] = vertex_buffer_size;
					vertex_buffer_size++;
				}
				index_buffer_size++;
			}
			index_offset += fv;
			// per-face material
			//            mesh.material_ids[f];
		}

		vertex_buffers.push_back(std::make_shared<cg::resource<cg::vertex>>(vertex_buffer_size));
		index_buffers.push_back(std::make_shared<cg::resource<unsigned int>>(index_buffer_size));
	}

	// second loop
	// Loop over shapes
	for (size_t s = 0; s < shapes.size(); s++) {
		// Loop over faces(polygon)
		size_t index_offset = 0;
		unsigned int vertex_buffer_id = 0;
		unsigned int index_buffer_id = 0;
		auto vertex_buffer = vertex_buffers[s];
		auto index_buffer = index_buffers[s];

		std::map<std::tuple<int, int, int>, unsigned int> index_map;
		const auto& mesh = shapes[s].mesh;
		for (size_t f = 0; f < mesh.num_face_vertices.size(); f++) {
			int fv = mesh.num_face_vertices[f];
			// Loop over vertices in the face.
			float3 normal;
			if (mesh.indices[index_offset].normal_index < 0) {
				auto a_id = mesh.indices[index_offset];
				auto b_id = mesh.indices[index_offset + 1];
				auto c_id = mesh.indices[index_offset + 2];