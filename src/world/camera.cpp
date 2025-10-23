#define _USE_MATH_DEFINES

#include "camera.h"

#include "utils/error_handler.h"

#include <math.h>


using namespace cg::world;

cg::world::camera::camera() : theta(0.f), phi(0.f), height(1080.f), width(1920.f),
							  aspect_ratio(1920.f / 1080.f), angle_of_view(1.04719f),
							  z_near(0.001f), z_far(100.f), position(float3{0.f, 0.f, 0.f})
{
}

cg::world::camera::~camera() {}

void cg::world::camera::set_position(float3 in_position)
{
	// TODO Lab: 1.04 Implement `cg::world::camera` class
	position = in_position;
}

void cg::world::camera::set_theta(float in_theta)
{
	// TODO Lab: 1.04 Implement `cg::world::camera` class
	theta = in_theta * static_cast<float>(M_PI) / 180.f;
}

void cg::world::camera::set_phi(float in_phi)
{
	// TODO Lab: 1.04 Implement `cg::world::camera` class
	phi = in_phi * static_cast<float>(M_PI) / 180.f;
}

void cg::world::camera::set_angle_of_view(float in_aov)
{
	// TODO Lab: 1.04 Implement `cg::world::camera` class
	angle_of_view = in_aov * static_cast<float>(M_PI) / 180.f;
}

void cg::world::camera::set_height(float in_height)
{
	// TODO Lab: 1.04 Implement `cg::world::camera` class
	height = in_height;
	aspect_ratio = width / height;
}

void cg::world::camera::set_width(float in_width)
{
	// TODO Lab: 1.04 Implement `cg::world::camera` class
	width = in_width;
	aspect_ratio = width / height;
}

void cg::world::camera::set_z_near(float in_z_near)
{
	// TODO Lab: 1.04 Implement `cg::world::camera` class
	z_near = in_z_near;
}

void cg::world::camera::set_z_far(float in_z_far)
{
	// TODO Lab: 1.04 Implement `cg::world::camera` class
	z_far = in_z_far;
}

const float4x4 cg::world::camera::get_view_matrix() const
{
	// TODO Lab: 1.04 Implement `cg::world::camera` class
	// Классический look-at: оси камеры и сдвиг
	const float3 up_world{0.f, 1.f, 0.f};
	const float3 dir = get_direction();
	const float3 right = normalize(cross(up_world, dir));
	const float3 up = cross(dir, right);

	// Праворукая система; третья строка соответствует -dir
	return float4x4{
		{ right.x,        right.y,        right.z,        -dot(right, position) },
		{ up.x,           up.y,           up.z,           -dot(up, position)    },
		{ -dir.x,         -dir.y,         -dir.z,          dot(dir, position)   },
		{ 0.f,            0.f,            0.f,             1.f                  }
	};
}

#ifdef DX12
const DirectX::XMMATRIX cg::world::camera::get_dxm_view_matrix() const
{
	// TODO Lab: 3.08 Implement `get_dxm_view_matrix`, `get_dxm_projection_matrix`, and `get_dxm_mvp_matrix` methods of `camera`
	return  DirectX::XMMatrixIdentity();
}

const DirectX::XMMATRIX cg::world::camera::get_dxm_projection_matrix() const
{
	// TODO Lab: 3.08 Implement `get_dxm_view_matrix`, `get_dxm_projection_matrix`, and `get_dxm_mvp_matrix` methods of `camera`
	return  DirectX::XMMatrixIdentity();
}

const DirectX::XMMATRIX camera::get_dxm_mvp_matrix() const
{
	// TODO Lab: 3.08 Implement `get_dxm_view_matrix`, `get_dxm_projection_matrix`, and `get_dxm_mvp_matrix` methods of `camera`
	return  DirectX::XMMatrixIdentity();
}
#endif

const float4x4 cg::world::camera::get_projection_matrix() const
{
	// TODO Lab: 1.04 Implement `cg::world::camera` class
	// Праворукая перспектива с NDC z в [-1,1]
	const float f = 1.f / tanf(angle_of_view * 0.5f);
	return float4x4{
		{ f / aspect_ratio, 0.f, 0.f,  0.f },
		{ 0.f,               f,   0.f,  0.f },
		{ 0.f,               0.f,  z_far / (z_near - z_far), -1.f },
		{ 0.f,               0.f, (z_far * z_near) / (z_near - z_far), 0.f }
	};
}

const float3 cg::world::camera::get_position() const
{
	// TODO Lab: 1.04 Implement `cg::world::camera` class
	return position;
}

const float3 cg::world::camera::get_direction() const
{
	// TODO Lab: 1.04 Implement `cg::world::camera` class
	// Направление из yaw=theta и pitch=phi; при theta=0, phi=0 — взгляд вдоль -Z
	return float3{
		sinf(theta) * cosf(phi),
		sinf(phi),
		-cosf(theta) * cosf(phi)
	};
}

const float3 cg::world::camera::get_right() const
{
	// TODO Lab: 1.04 Implement `cg::world::camera` class
	return normalize(cross(float3{0.f, 1.f, 0.f}, get_direction()));
}

const float3 cg::world::camera::get_up() const
{
	// TODO Lab: 1.04 Implement `cg::world::camera` class
	return cross(get_direction(), get_right());
}
const float camera::get_theta() const
{
	// TODO Lab: 1.04 Implement `cg::world::camera` class
	return theta;
}
const float camera::get_phi() const
{
	// TODO Lab: 1.04 Implement `cg::world::camera` class
	return phi;
}
