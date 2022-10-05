#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
using namespace std;

#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>

#include "Shader.hpp"

// TODO(hobrzut): Move it to a better place.
template<typename Lambda>
struct Scope_guard
{
   Scope_guard(Lambda &&lambda) : lambda(std::forward<Lambda>(lambda)) {}
   ~Scope_guard() { lambda(); }

   Lambda lambda;
};

#define DO_CONCAT(a, b) a##b
#define CONCAT(a, b) DO_CONCAT(a, b)
#define UNIQUENAME( prefix ) CONCAT(prefix, __COUNTER__)
#define defer Scope_guard UNIQUENAME(sg) = [&]()

namespace Shader
{

static GLuint
compile(const char *shader_code, GLenum shader_type)
{
   GLuint id = glCreateShader(shader_type);
   if (!id)
      return 0;

   glShaderSource(id, 1, &shader_code, 0);
   glCompileShader(id);

   GLint compile_status;
   glGetShaderiv(id, GL_COMPILE_STATUS, &compile_status);
   if (!compile_status)
   {
      glDeleteShader(id);
      return 0;
   }

   return id;
}

static char *
read_file(FILE *file)
{
   fseek(file, 0, SEEK_END);
   long length = ftell(file);
   char *contents = (char *)malloc(length+1);
   rewind(file);
   fread(contents, 1, length, file);
   contents[length] = 0;

   return contents;
}

GLuint
compile(const char *vertex_code, const char *fragment_code)
{
   GLuint program_id = glCreateProgram();
   if (!program_id)
      return 0;

   GLuint vertex_id = compile(vertex_code, GL_VERTEX_SHADER);
   GLuint fragment_id = compile(fragment_code, GL_FRAGMENT_SHADER);

   defer {
      glDeleteShader(vertex_id);
      glDeleteShader(fragment_id);
   };

   glAttachShader(program_id, vertex_id);
   glAttachShader(program_id, fragment_id);
   glLinkProgram(program_id);

   GLint link_status;
   glGetProgramiv(program_id, GL_LINK_STATUS, &link_status);
   if (!link_status)
   {
      glDeleteProgram(program_id);
      return 0;
   }

   glDetachShader(program_id, vertex_id);
   glDetachShader(program_id, fragment_id);

   return program_id;
}

GLuint
load(const char *vertex_shader_path, const char *fragment_shader_path)
{
   FILE *vertex_file = fopen(vertex_shader_path, "r");
   if (!vertex_file)
      return 0;

   char *vertex_code = read_file(vertex_file);
   defer { free(vertex_code); };

   fclose(vertex_file);

   FILE *fragment_file = fopen(fragment_shader_path, "r");
   if (!fragment_file)
      return 0;

   char *fragment_code = read_file(fragment_file);
   defer { free(fragment_code); };

   fclose(fragment_file);

   return compile(vertex_code, fragment_code);
}

} // namespace Shader
