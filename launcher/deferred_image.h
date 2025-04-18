#pragma once

#include "Texture.h"
#include "renderer.h"

class deferred_image {
public:
	deferred_image() = default;
	deferred_image(const std::string &p) : path(p) { }
	deferred_image(const deferred_image&) = delete;
	deferred_image(deferred_image&&) = default;
	deferred_image &operator=(deferred_image&&) = default;
	operator bool() const
	{
		return image != null_handle || !path.empty();
	}

	GLuint get() const
	{
		if (!path.empty()) {
            image = GfxRenderer->Fetch_Texture(path, true);
			path.clear();
		}

		if (image != null_handle) {
            auto &tex = GfxRenderer->Texture(image);
			tex.create();

			if (tex.get_is_ready())
				return tex.get_id();
		}

		return -1;
	}

	glm::ivec2 size() const
	{
		if (image != null_handle) {
            auto &tex = GfxRenderer->Texture(image);
			return glm::ivec2(tex.get_width(), tex.get_height());
		}
		return glm::ivec2();
	}

private:
	mutable std::string path;
	mutable texture_handle image = 0;
};
