#include "shader.h"

#include <stdio.h>
#include <malloc.h>
#include <utility>

#include "error.h"

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

namespace Graphics
{

static GLuint
compile_shader(const char *shader_code, GLenum shader_type)
{
   GL_CALL(GLuint id = glCreateShader(shader_type));
   if (!id)
      return 0;

   GL_CALL(glShaderSource(id, 1, &shader_code, 0));
   GL_CALL(glCompileShader(id));

   GLint compile_status;
   GL_CALL(glGetShaderiv(id, GL_COMPILE_STATUS, &compile_status));
   if (compile_status == GL_FALSE)
   {
      GLint info_log_length;
      GL_CALL(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &info_log_length));

      char *info_log = (char *)malloc(info_log_length * sizeof(char));
      defer { free(info_log); };

      // TODO(hobrzut): Remove logging.
      GL_CALL(glGetShaderInfoLog(id, info_log_length, 0, info_log));
      fprintf(stderr, "%.*s", info_log_length, info_log);

      GL_CALL(glDeleteShader(id));
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
compile_shaders(const char *vertex_code, const char *fragment_code)
{
   GL_CALL(GLuint program_id = glCreateProgram());
   if (!program_id)
      return 0;

   GLuint vertex_id = compile_shader(vertex_code, GL_VERTEX_SHADER);
   if (!vertex_id)
      return 0;

   defer { GL_CALL(glDeleteShader(vertex_id)); };

   GLuint fragment_id = compile_shader(fragment_code, GL_FRAGMENT_SHADER);
   if (!fragment_id)
      return 0;

   defer { GL_CALL(glDeleteShader(fragment_id)); };

   GL_CALL(glAttachShader(program_id, vertex_id));
   GL_CALL(glAttachShader(program_id, fragment_id));
   GL_CALL(glLinkProgram(program_id));

   GLint link_status;
   GL_CALL(glGetProgramiv(program_id, GL_LINK_STATUS, &link_status));
   if (link_status == GL_FALSE)
   {
      GL_CALL(glDeleteProgram(program_id));
      return 0;
   }

   GL_CALL(glDetachShader(program_id, vertex_id));
   GL_CALL(glDetachShader(program_id, fragment_id));

   return program_id;
}

GLuint
load_shaders(const char *vertex_shader_path, const char *fragment_shader_path)
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

   return compile_shaders(vertex_code, fragment_code);
}

} // namespace Graphics
