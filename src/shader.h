#ifndef SHADER_HPP
#define SHADER_HPP

#include <GL/glew.h>

namespace Graphics
{

GLuint
compile_shaders(const char *vertex_code, const char *fragment_code);
GLuint
load_shaders(const char *vertex_file_path, const char *fragment_file_path);

}

#endif
