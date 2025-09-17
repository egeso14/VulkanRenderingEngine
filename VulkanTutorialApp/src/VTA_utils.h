#pragma once
//libs

#include "tiny_obj_loader.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <functional>

namespace VTA {


	template <typename T, typename... Rest>
	void hashCombine(std::size_t& seed, const T& v, const Rest&... rest) {
		seed ^= std::hash<T>{}(v)+0x9e3779b9 + (seed << 6) + (seed >> 2);
		(hashCombine(seed, rest), ...);
	};

}