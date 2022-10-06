#ifndef ERROR_H
#define ERROR_H

#include <assert.h>
#include <GL/glew.h>

#ifndef NDEBUG
#define GL_CALL(x) while (glGetError() != GL_NO_ERROR); x; assert(gl_log_error(#x, __FILE__, __LINE__));
#else
#define GL_CALL(x) x
#endif

bool
gl_log_error(const char* call, const char* file, int line);

#endif
