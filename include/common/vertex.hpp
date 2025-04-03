#ifndef _VERTEX_H_
#define _VERTEX_H_

#include "common/typedefs.hpp"

namespace engine {

struct Vertex {
	Vec3f position;
	Vec3f normal;
	Vec3f texCoord;

	Vertex() = default;

	Vertex(const cy::Vec3f &pos, const cy::Vec3f &norm, const cy::Vec3f &tex)
		: position(pos), normal(norm), texCoord(tex) {}

	bool operator==(const Vertex &other) const {
		return this->position == other.position &&
			   this->normal == other.normal && this->texCoord == other.texCoord;
	}
};

} // namespace engine

// https://johnfarrier.com/unlocking-the-power-of-stdhash-in-c-programming/
namespace std {
template <> struct hash<cy::Vec3f> {
	size_t operator()(const cy::Vec3f &v) const {
		size_t h1 = std::hash<float>()(v.x);
		size_t h2 = std::hash<float>()(v.y);
		size_t h3 = std::hash<float>()(v.z);
		return h1 ^ (h2 << 1) ^ (h3 << 2);
	}
};
template <> struct hash<engine::Vertex> {
	size_t operator()(const engine::Vertex &v) const {
		size_t h1 = std::hash<cy::Vec3f>()(v.position);
		size_t h2 = std::hash<cy::Vec3f>()(v.normal);
		size_t h3 = std::hash<cy::Vec3f>()(v.texCoord);
		return h1 ^ (h2 * 31) ^ (h3 * 131);
	}
};
} // namespace std

#endif
