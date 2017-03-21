#pragma once
#ifndef __Texture__
#define __Texture__

#define GLEW_STATIC
#include <GL/glew.h>

#include <string>

class Texture
{
public:
	Texture();
	virtual ~Texture();
	void setFilename(const std::string &f) { filename = f; }
	void setName(const std::string &n) { name = n; }
	const std::string &getName() const { return name; }
	void init();
	void setUnit(GLint u) { unit = u; }
	GLint getUnit() const { return unit; }
	void setHandle(GLint h) { handle = h; }
	GLint getHandle() const { return handle; }
	void bind(GLint handle);
	void bind();
	void unbind();
	void setWrapModes(GLint wrapS, GLint wrapT); // Must be called after init()
	GLint getID() const { return tid;}
private:
	std::string filename;
	int width;
	int height;
	GLuint tid;
	GLint unit;
	std::string name;
	GLint handle;
};

#endif
