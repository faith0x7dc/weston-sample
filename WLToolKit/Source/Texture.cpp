#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string>

#include "Common.hpp"
#include "WindowEGL.hpp"
#include "Texture.hpp"

namespace WLToolKit {

static bool IsFileExists(const char *filename);

struct TextureImpl {
	TextureImpl() : width(0), height(0), stride(0), pixels(NULL), bLoaded(false) {}

	int width;
	int height;
	int stride;
	unsigned char *pixels;

	bool bLoaded;

	GLuint texture;
};

Texture::Texture(const char *filename)
{
	m_pImpl = new TextureImpl;

	Load(filename);
}

Texture::~Texture()
{
	delete m_pImpl;
}

bool
Texture::IsLoaded()
{
	return m_pImpl->bLoaded;
}

bool
Texture::Load(const char *filename)
{
	if (!IsFileExists(filename)) {
		Release();
		return false;
	}

	Release();

	cairo_surface_t* surface = cairo_image_surface_create_from_png(filename);

	cairo_format_t format = cairo_image_surface_get_format(surface);
	if ((format != CAIRO_FORMAT_ARGB32) && (format != CAIRO_FORMAT_RGB24)) {
		fprintf(stderr, "[WLToolKit] ERR: format(%d) is not supported\n", format);
		return false;
	}

	m_pImpl->width = cairo_image_surface_get_width(surface);
	m_pImpl->height = cairo_image_surface_get_height(surface);
	m_pImpl->stride = cairo_image_surface_get_stride(surface);

	unsigned char* data = cairo_image_surface_get_data(surface);

	m_pImpl->pixels = new unsigned char[m_pImpl->stride * m_pImpl->height];

	for (int y = 0; y < m_pImpl->height; y++) {
		for (int x = 0; x < m_pImpl->width; x++) {
			int idx = (y * m_pImpl->width + x) << 2;

			m_pImpl->pixels[idx + 0] = data[idx + 2];
			m_pImpl->pixels[idx + 1] = data[idx + 1];
			m_pImpl->pixels[idx + 2] = data[idx + 0];
			m_pImpl->pixels[idx + 3] = data[idx + 3];
		}
	}

	cairo_surface_destroy(surface);

	glGenTextures(1, &m_pImpl->texture);
	glBindTexture(GL_TEXTURE_2D, m_pImpl->texture);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_pImpl->width, m_pImpl->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_pImpl->pixels);

	glBindTexture(GL_TEXTURE_2D, 0);

#if 0
	fprintf(stderr, "[WLToolKit] DBG: filename=%s\n", filename);
	fprintf(stderr, "[WLToolKit] DBG: width=%d\n", m_pImpl->width);
	fprintf(stderr, "[WLToolKit] DBG: height=%d\n", m_pImpl->height);
	fprintf(stderr, "[WLToolKit] DBG: stride=%d\n", m_pImpl->stride);
	fprintf(stderr, "[WLToolKit] DBG: format=%d\n", format);
	fprintf(stderr, "[WLToolKit] DBG: texture=%d\n", m_pImpl->texture);
#endif

	m_pImpl->bLoaded = true;

	return true;
}

void
Texture::Release()
{
	if (m_pImpl->bLoaded) {
		m_pImpl->width = 0;
		m_pImpl->height = 0;
		m_pImpl->stride = 0;

		delete[] m_pImpl->pixels;
		m_pImpl->pixels = NULL;

		m_pImpl->bLoaded = false;

		m_pImpl->texture = 0;
	}
}

int
Texture::GetWidth()
{
	return m_pImpl->width;
}

int
Texture::GetHeight()
{
	return m_pImpl->height;
}

int
Texture::GetStride()
{
	return m_pImpl->stride;
}

unsigned char *
Texture::GetPixels()
{
	return m_pImpl->pixels;
}

void
Texture::Draw(WindowEGL *window, int x, int y)
{
	if (!IsLoaded())
		return;

	GLfloat left	= ((float)x / window->GetWidth()) * 2.0f - 1.0f;
	GLfloat top		= ((float)y / window->GetHeight()) * 2.0f - 1.0f;
	GLfloat right	= ((float)(x + GetWidth()) / window->GetWidth()) * 2.0f - 1.0f;
	GLfloat bottom	= ((float)(y + GetHeight()) / window->GetHeight()) * 2.0f - 1.0f;
	top = -top;
	bottom = -bottom;

	GLfloat vertices[] = {
		left,  top,
		left,  bottom,
		right, top,
		right, bottom,
	};

	GLfloat texCoords[] = {
		0.0f, 0.0f,		// left top
		0.0f, 1.0f,		// left bottom
		1.0f, 0.0f,		// right top
		1.0f, 1.0f,		// right bottom
	};

	GLfloat matrix[] = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};

	glUniformMatrix4fv(window->GetRotationUniform(), 1, GL_FALSE, matrix);

	glVertexAttribPointer(window->GetVertexAttribute(),		2, GL_FLOAT, GL_FALSE, 0, vertices);
	glVertexAttribPointer(window->GetTexCoordAttribute(),	2, GL_FLOAT, GL_FALSE, 0, texCoords);

	glEnableVertexAttribArray(window->GetVertexAttribute());
	glEnableVertexAttribArray(window->GetTexCoordAttribute());
	glEnableVertexAttribArray(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, m_pImpl->texture);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisable(GL_BLEND);

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisableVertexAttribArray(GL_TEXTURE_2D);
	glDisableVertexAttribArray(window->GetTexCoordAttribute());
	glDisableVertexAttribArray(window->GetVertexAttribute());
}

void
Texture::Draw(WindowEGL *window, int x, int y, float scale)
{
	if (!IsLoaded())
		return;

#if 1
	GLfloat left	= ((float)x / window->GetWidth()) * 2.0f - 1.0f;
	GLfloat top		= ((float)y / window->GetHeight()) * 2.0f - 1.0f;
	GLfloat right	= ((float)(x + GetWidth()) / window->GetWidth()) * 2.0f - 1.0f;
	GLfloat bottom	= ((float)(y + GetHeight()) / window->GetHeight()) * 2.0f - 1.0f;
	top = -top;
	bottom = -bottom;

	left *= scale;
	top *= scale;
	right *= scale;
	bottom *= scale;

	GLfloat vertices[] = {
		left,  top,
		left,  bottom,
		right, top,
		right, bottom,
	};

#else

	GLfloat dx = (float)x / window->GetWidth();
	GLfloat dy = (float)y / window->GetHeight();

	GLfloat w = 2.0f * ((float)GetWidth() / window->GetWidth());
	GLfloat h = 2.0f * ((float)GetHeight() / window->GetHeight());

	GLfloat vertices[] = {
		-1.0f,		1.0f,
		-1.0f,		1.0f - h,
		-1.0f + w,	1.0f,
		-1.0f + w,	1.0f - h,
	};
#endif

	GLfloat texCoords[] = {
		0.0f, 0.0f,		// left top
		0.0f, 1.0f,		// left bottom
		1.0f, 0.0f,		// right top
		1.0f, 1.0f,		// right bottom
	};

	GLfloat matrix[] = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};

#if 0
	matrix[3] = dx;
	matrix[7] = dy;
#endif

#if 0
	matrix[0] = scale;
	matrix[5] = scale;
#endif

	glUniformMatrix4fv(window->GetRotationUniform(), 1, GL_FALSE, matrix);

	glVertexAttribPointer(window->GetVertexAttribute(),		2, GL_FLOAT, GL_FALSE, 0, vertices);
	glVertexAttribPointer(window->GetTexCoordAttribute(),	2, GL_FLOAT, GL_FALSE, 0, texCoords);

	glEnableVertexAttribArray(window->GetVertexAttribute());
	glEnableVertexAttribArray(window->GetTexCoordAttribute());
	glEnableVertexAttribArray(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, m_pImpl->texture);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisable(GL_BLEND);

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisableVertexAttribArray(GL_TEXTURE_2D);
	glDisableVertexAttribArray(window->GetTexCoordAttribute());
	glDisableVertexAttribArray(window->GetVertexAttribute());
}

static bool
IsFileExists(const char *filename)
{
	int ret;
	struct stat st;

	ret = stat(filename, &st);

	return (ret == 0);
}

} // End-of-namespace WLToolKit

