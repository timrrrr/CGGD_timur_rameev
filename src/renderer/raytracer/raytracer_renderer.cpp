#include "raytracer_renderer.h"

#include "utils/resource_utils.h"

#include <iostream>

void cg::renderer::ray_tracing_renderer::init()
{
	model = std::make_shared<cg::world::model>();
	model->load_obj(settings->model_path);

	camera = std::make_shared<cg::world::camera>();

	camera->set_height(static_cast<float>(settings->height));
	camera->set_width(static_cast<float>(settings->width));

	camera->set_position(float3{ settings->camera_position[0],
								settings->camera_position[1],
								settings->camera_position[2] });
	camera->set_theta(settings->camera_theta);
	camera->set_phi(settings->camera_phi);

	camera->set_angle_of_view(settings->camera_angle_of_view);
	camera->set_z_far(settings->camera_z_far);
	camera->set_z_near(settings->camera_z_near);

	render_target = std::make_shared<cg::resource<cg::unsigned_color>>(
			settings->width, settings->height);

	raytracer =
			std::make_shared<cg::renderer::raytracer<cg::vertex, cg::unsigned_color>>();

	raytracer->set_render_target(render_target);
	raytracer->set_viewport(settings->width, settings->height);
	raytracer->set_vertex_buffers(model->get_vertex_buffers());
	raytracer->set_index_buffers(model->get_index_buffers());
}

void cg::renderer::ray_tracing_renderer::destroy() {}

void cg::renderer::ray_tracing_renderer::update() {}

void cg::renderer::ray_tracing_renderer::render()
{
	raytracer->clear_render_target({ 0, 0, 0 });

	raytracer->miss_shader = [](const ray& ray)
	{
		payload payload;
		payload.color = { ray.direction.x * 0.5f + 0.5f, ray.direction.y * 0.5f + 0.5f,
						 ray.direction.z * 0.5f + 0.5f};
		return payload;
	};

	raytracer->closest_hit_shader = [](const ray& ray, payload& payload,
									   const triangle<cg::vertex>& triangle)
	{
		payload.color = cg::color::from_float3(triangle.ambient);
		return payload;
	};

	raytracer->build_acceleration_structure();


	raytracer->ray_generation(
			camera->get_position(), camera->get_direction(),
			camera->get_right(), camera->get_up(),
			settings->raytracing_depth, settings-> accumulation_num);
	cg::utils::save_resource(*render_target, settings->result_path);
}