bool
gl_log_error(const char *call, const char *file, int line)
{
   GLenum err;
   bool no_error = true;

   while ((err = glGetError()) != GL_NO_ERROR)
   {
      const char *msg = (const char *)gluErrorString(err);
      fprintf(stderr, "OpenGL error [%d] after '%s' at %s:%d: %s\n", err, call, file, line, msg);
      no_error = false;
   }

   return no_error;
}
