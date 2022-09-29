#pragma once

namespace Shader
{

GLuint
compile(const char *vertex_code, const char *fragment_code);
GLuint
load(const char *vertex_file_path, const char *fragment_file_path);

}
