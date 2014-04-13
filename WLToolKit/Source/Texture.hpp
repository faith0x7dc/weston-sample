#ifndef WL_TOOLKIT_TEXTURE_HPP
#define WL_TOOLKIT_TEXTURE_HPP

namespace WLToolKit {

class WindowEGL;
struct TextureImpl;

class Texture {
public:
	Texture(const char *filename);
	virtual ~Texture();

	bool IsLoaded();

	bool Load(const char *filename);
	void Release();

	int GetWidth();
	int GetHeight();
	int GetStride();

	unsigned char *GetPixels();

	void Draw(WindowEGL *window, int x, int y);
	void Draw(WindowEGL *window, int x, int y, float scale);

protected:
	struct TextureImpl *m_pImpl;
}; // End-of-class Texture

} // End-of-namespace WLToolKit

#endif /* WL_TOOLKIT_TEXTURE_HPP */
