#include "rasterizer_renderer.h"

#include "utils/resource_utils.h"
#include "utils/timer.h"


void cg::renderer::rasterization_renderer::init()
{
	// TODO Lab: 1.02 Implement image clearing & saving in `cg::renderer::rasterization_renderer` class
	// TODO Lab: 1.03 Adjust `cg::renderer::rasterization_renderer` and `cg::renderer::renderer` classes to consume `cg::world::model`
	// TODO Lab: 1.04 Setup an instance of camera `cg::world::camera` class in `cg::renderer::renderer` and `cg::renderer::rasterization_renderer` 
	// TODO Lab: 1.06 Add depth buffer in `cg::renderer::rasterization_renderer`
	rasterizer = std::make_shared<cg::renderer::rasterizer<cg::vertex, cg::unsigned_color>>();
	rasterizer->set_viewport(settings->width, settings->height); 

	// Создать render target и depth buffer и привязать их к растеризатору 
	render_target = std::make_shared<cg::resource<cg::unsigned_color>>(settings->width, settings->height);
	depth_buffer = std::make_shared<cg::resource<float>>(settings->width, settings->height);
	rasterizer->set_render_target(render_target, depth_buffer); 

	// Загрузить модель из настроек (дублирует базовый renderer::load_model, но это локально для этого рендера) 
	model = std::make_shared<cg::world::model>();
	model->load_obj(settings->model_path); 

	// Настроить камеру из настроек (дублирует renderer::load_camera для автономности) 
	camera = std::make_shared<cg::world::camera>();
	camera->set_height(static_cast<float>(settings->height));
	camera->set_width(static_cast<float>(settings->width));
	float3 pos{0.f,1.f,5.f};
	if (settings->camera_position.size() >= 3) {
		pos = float3{settings->camera_position[0], settings->camera_position[1], settings->camera_position[2]};
	}
	camera->set_position(pos);
	camera->set_phi(settings->camera_phi);
	camera->set_theta(settings->camera_theta);
	camera->set_angle_of_view(settings->camera_angle_of_view);
	camera->set_z_near(settings->camera_z_near);
	camera->set_z_far(settings->camera_z_far); 
}
void cg::renderer::rasterization_renderer::render()
{
	// TODO Lab: 1.02 Implement image clearing & saving in `cg::renderer::rasterization_renderer` class
	// TODO Lab: 1.04 Implement `vertex_shader` lambda for the instance of `cg::renderer::rasterizer`
	// TODO Lab: 1.05 Implement `pixel_shader` lambda for the instance of `cg::renderer::rasterizer`
	// TODO Lab: 1.03 Adjust `cg::renderer::rasterization_renderer` and `cg::renderer::renderer` classes to consume `cg::world::model`
	float4x4 matrix = mul(
		camera->get_projection_matrix(),
		camera->get_view_matrix(),
		model->get_world_matrix()
	);

	// Привязать лямбды шейдеров 
	rasterizer->vertex_shader = [matrix](float4 vertex, cg::vertex vertex_data) {
		float4 clip = mul(matrix, vertex); // позиция в clip‑пространстве 
		return std::make_pair(clip, vertex_data); // пробрасываем атрибуты без изменений
	};

	rasterizer->pixel_shader = [](const cg::vertex&, const float) -> cg::color {
	return cg::color::from_float3(float3{1.f, 1.f, 1.f});
	};

	// Очистка цветового и глубинного буфера 
	rasterizer->clear_render_target(cg::unsigned_color{0, 255, 0}, 1.0f);
	// Отрисовка по всем shape модели: задаем VB/IB и вызываем draw 
	for (size_t shape = 0; shape < model->get_index_buffers().size(); ++shape)
	{
		rasterizer->set_vertex_buffer(model->get_vertex_buffers()[shape]); 
		rasterizer->set_index_buffer(model->get_index_buffers()[shape]);   
		rasterizer->draw(model->get_index_buffers()[shape]->count(), 0);   
	}

	// Сохранить результат в файл из настроек 
	cg::utils::save_resource(*render_target, settings->result_path);
}

void cg::renderer::rasterization_renderer::destroy() {}

void cg::renderer::rasterization_renderer::update() {}
