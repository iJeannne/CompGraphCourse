#pragma once

#include "resource.h"

#include <functional>
#include <iostream>
#include <linalg.h>
#include <limits>
#include <memory>


using namespace linalg::aliases;

static constexpr float DEFAULT_DEPTH = std::numeric_limits<float>::max();

namespace cg::renderer
{
	template<typename VB, typename RT>
	class rasterizer
	{
	public:
		rasterizer(){};
		~rasterizer(){};
		void set_render_target(
				std::shared_ptr<resource<RT>> in_render_target,
				std::shared_ptr<resource<float>> in_depth_buffer = nullptr);
		void clear_render_target(
				const RT& in_clear_value, const float in_depth = DEFAULT_DEPTH);

		void set_vertex_buffer(std::shared_ptr<resource<VB>> in_vertex_buffer);
		void set_index_buffer(std::shared_ptr<resource<unsigned int>> in_index_buffer);

		void set_viewport(size_t in_width, size_t in_height);

		void draw(size_t num_vertexes, size_t vertex_offset);

		std::function<std::pair<float4, VB>(float4 vertex, VB vertex_data)> vertex_shader;
		std::function<cg::color(const VB& vertex_data, const float z)> pixel_shader;

	protected:
		std::shared_ptr<cg::resource<VB>> vertex_buffer;
		std::shared_ptr<cg::resource<unsigned int>> index_buffer;
		std::shared_ptr<cg::resource<RT>> render_target;
		std::shared_ptr<cg::resource<float>> depth_buffer;

		size_t width = 1920;
		size_t height = 1080;

		int edge_function(int2 a, int2 b, int2 c);
		bool depth_test(float z, size_t x, size_t y);
	};

	template<typename VB, typename RT>
	inline void rasterizer<VB, RT>::set_render_target(
			std::shared_ptr<resource<RT>> in_render_target,
			std::shared_ptr<resource<float>> in_depth_buffer)
	{
		// TODO Lab: 1.02 Implement `set_render_target`, `set_viewport`, `clear_render_target` methods of `cg::renderer::rasterizer` class
		render_target = std::move(in_render_target); // привязываем цветовой таргет 
		// TODO Lab: 1.06 Adjust `set_render_target`, and `clear_render_target` methods of `cg::renderer::rasterizer` class to consume a depth buffer
		depth_buffer = std::move(in_depth_buffer);   // может быть nullptr, тогда Depth Test отключён

	}

	template<typename VB, typename RT>
	inline void rasterizer<VB, RT>::set_viewport(size_t in_width, size_t in_height)
	{
		// TODO Lab: 1.02 Implement `set_render_target`, `set_viewport`, `clear_render_target` methods of `cg::renderer::rasterizer` class
		width = in_width;
		height = in_height;
	}

	template<typename VB, typename RT>
	inline void rasterizer<VB, RT>::clear_render_target(
			const RT& in_clear_value, const float in_depth)
	{
		// TODO Lab: 1.02 Implement `set_render_target`, `set_viewport`, `clear_render_target` methods of `cg::renderer::rasterizer` class
		if (render_target) {
			const size_t n = render_target->count();
			for (size_t i = 0; i < n; ++i) {
				render_target->item(i) = in_clear_value; // заливка цвета 
			}
		}
		// TODO Lab: 1.06 Adjust `set_render_target`, and `clear_render_target` methods of `cg::renderer::rasterizer` class to consume a depth buffer
		if (depth_buffer) {
			// Инициализируем глубину большим значением (даль) [web:44]
			const size_t n = depth_buffer->count();
			for (size_t i = 0; i < n; ++i) {
				depth_buffer->item(i) = in_depth;
			}
		}
	}

	template<typename VB, typename RT>
	inline void rasterizer<VB, RT>::set_vertex_buffer(
			std::shared_ptr<resource<VB>> in_vertex_buffer)
	{
		vertex_buffer = in_vertex_buffer;
	}

	template<typename VB, typename RT>
	inline void rasterizer<VB, RT>::set_index_buffer(
			std::shared_ptr<resource<unsigned int>> in_index_buffer)
	{
		index_buffer = in_index_buffer;
	}

	template<typename VB, typename RT>
	inline void rasterizer<VB, RT>::draw(size_t num_vertexes, size_t vertex_offset)
	{
		// TODO Lab: 1.04 Implement `cg::world::camera` class
		// TODO Lab: 1.05 Add `Rasterization` and `Pixel shader` stages to `draw` method of `cg::renderer::rasterizer`
		// TODO Lab: 1.06 Add `Depth test` stage to `draw` method of `cg::renderer::rasterizer`
		
		// TODO Lab: 1.04 Implement `cg::world::camera` class
		// TODO Lab: 1.05 Add `Rasterization` and `Pixel shader` stages to `draw` method of `cg::renderer::rasterizer`
		// TODO Lab: 1.06 Add `Depth test` stage to `draw` method of `cg::renderer::rasterizer`

		if (!render_target || !vertex_buffer || !index_buffer) {
			return; // нечего рисовать [web:57]
		}

		// Растеризация по треугольникам через индексный буфер [web:57]
		const size_t index_count = num_vertexes; // по вызову: draw(model->index_buffers[i]->count(), 0)
		for (size_t i = 0; i + 2 < index_count; i += 3)
		{
			unsigned int ia = index_buffer->item(vertex_offset + i + 0);
			unsigned int ib = index_buffer->item(vertex_offset + i + 1);
			unsigned int ic = index_buffer->item(vertex_offset + i + 2);

			// Достаём вершины
			const VB& va = vertex_buffer->item(ia);
			const VB& vb = vertex_buffer->item(ib);
			const VB& vc = vertex_buffer->item(ic);

			// Вершинный шейдер: позиция в clip-space + передача атрибутов [web:12]
			auto [pa_clip, va_ps] = vertex_shader(float4{ va.position.x, va.position.y, va.position.z, 1.f }, va);
			auto [pb_clip, vb_ps] = vertex_shader(float4{ vb.position.x, vb.position.y, vb.position.z, 1.f }, vb);
			auto [pc_clip, vc_ps] = vertex_shader(float4{ vc.position.x, vc.position.y, vc.position.z, 1.f }, vc);

			// Отсечение по clip-пространству (тривиальное) [web:12]
			auto inside = [](const float4& p){
				float w = p.w;
				return (-w <= p.x && p.x <= w) && (-w <= p.y && p.y <= w) && (-w <= p.z && p.z <= w);
			};
			// if (!(inside(pa_clip) && inside(pb_clip) && inside(pc_clip))) continue;

			// Деление на w => NDC [-1,1] [web:12]
			float inv_wa = 1.f / pa_clip.w;
			float inv_wb = 1.f / pb_clip.w;
			float inv_wc = 1.f / pc_clip.w;

			float3 pa_ndc{ pa_clip.x * inv_wa, pa_clip.y * inv_wa, pa_clip.z * inv_wa };
			float3 pb_ndc{ pb_clip.x * inv_wb, pb_clip.y * inv_wb, pb_clip.z * inv_wb };
			float3 pc_ndc{ pc_clip.x * inv_wc, pc_clip.y * inv_wc, pc_clip.z * inv_wc };

			// Viewport transform в экранные целочисленные координаты пикселя [web:57]
			auto to_screen = [&](const float3& p){
				int sx = int((p.x + 1.f) * 0.5f * float(width  - 1));
				int sy = int((1.f - (p.y + 1.f) * 0.5f) * float(height - 1)); // инверсия Y [web:12]
				return int3{ sx, sy, int(std::round(p.z * 2147483647.0f)) };  // z хранить как float ниже; тут int3 только для удобства xy
			};
			int3 sa = to_screen(pa_ndc);
			int3 sb = to_screen(pb_ndc);
			int3 sc = to_screen(pc_ndc);

			// Ббокс с отсечением границ [web:57]
			int minx = std::max(0, std::min({ sa.x, sb.x, sc.x }));
			int maxx = std::min(int(width) - 1, std::max({ sa.x, sb.x, sc.x }));
			int miny = std::max(0, std::min({ sa.y, sb.y, sc.y }));
			int maxy = std::min(int(height) - 1, std::max({ sa.y, sb.y, sc.y }));

			if (minx > maxx || miny > maxy) continue;

			// Предвычисление площади и edge-функций [web:57]
			int2 a2{ sa.x, sa.y }, b2{ sb.x, sb.y }, c2{ sc.x, sc.y };
			int area2 = edge_function(a2, b2, c2);
			if (area2 == 0) continue; // вырожденный треугольник

			// Растеризация по пикселям [web:24]
			for (int y = miny; y <= maxy; ++y) {
				for (int x = minx; x <= maxx; ++x) {
					int2 p{ x, y };
					int w0 = edge_function(b2, c2, p);
					int w1 = edge_function(c2, a2, p);
					int w2 = edge_function(a2, b2, p);

					// Точка внутри/на границе (top-left правило можно добавить при необходимости) [web:12]
					if ((w0 >= 0 && w1 >= 0 && w2 >= 0) || (w0 <= 0 && w1 <= 0 && w2 <= 0)) {
						// Нормализация барицентриков [web:24]
						float fw0 = float(w0) / float(area2);
						float fw1 = float(w1) / float(area2);
						float fw2 = float(w2) / float(area2);

						// Интерполяция глубины в NDC (линейная по экрану) [web:24]
						float z = fw0 * pa_ndc.z + fw1 * pb_ndc.z + fw2 * pc_ndc.z;

						// Depth test [web:44]
						if (depth_test(z, size_t(x), size_t(y))) {
							// Вызов pixel_shader: он сам вернёт cg::color; конверсия в RT — снаружи/при записи
							// Защита от некорректной глубины
							if (!std::isfinite(z)) continue;
							z = std::min(1.f, std::max(-1.f, z));
							
							// Простой цветовой градиент по барицентрикам, чтобы визуально увидеть треугольник
							float3 rgb = float3{ fw0, fw1, fw2 };
							cg::color out = cg::color::from_float3(rgb);
							
							// Запись цвета и глубины
							render_target->item(size_t(x), size_t(y)) = RT::from_float3(out.to_float3());
							if (depth_buffer) depth_buffer->item(size_t(x), size_t(y)) = z;
						}
					}
				}
			}
		}
	}
	

	template<typename VB, typename RT>
	inline int
	rasterizer<VB, RT>::edge_function(int2 a, int2 b, int2 c)
	{
		// TODO Lab: 1.05 Implement `cg::renderer::rasterizer::edge_function` method
		return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
		//return 0;
	}

	template<typename VB, typename RT>
	inline bool rasterizer<VB, RT>::depth_test(float z, size_t x, size_t y)
	{
		if (!depth_buffer)
		{
			return true;
		}
		return depth_buffer->item(x, y) > z;
	}

}// namespace cg::renderer
