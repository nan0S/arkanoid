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

GLuint
load(const char *vertex_shader_path, const char *fragment_shader_path)
{
	GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

   FILE *vertex_shader_file = fopen(vertex_shader_path, "r");
   char *vertex_shader_code;
   if (vertex_shader_file)
   {
      fseek(vertex_shader_file, 0, SEEK_END);
      long vertex_shader_code_length = ftell(vertex_shader_file);
      fseek(vertex_shader_file, 0, SEEK_SET);
      vertex_shader_code = (char *)malloc(file_length+1);
      fread(vertex_shader_code, 1, vertex_shader_code_length, vertex_shader_file);
      vertex_shader_code[vertex_shader_code_length] = 0;
      fclose(vertex_shader_file);
   }
   else
      return 0;

   FILE *fragment_shader_file = fopen(fragment_shader_path, "r");
   char *fragment_shader_code;
   if (fragment_shader_file)
   {
      fseek(fragment_shader_file, 0, SEEK_END);
      long fragment_shader_code_length = ftell(fragment_shader_file);
      fseek(fragment_shader_file, 0, SEEK_SET);
      fragment_shader_code = (char *)malloc(file_length+1);
      fread(fragment_shader_code, 1, fragment_shader_code_length, fragment_shader_file);
      fragment_shader_code[fragment_shader_code_length] = 0;
      fclose(fragment_shader_file);
   }
   else
   {
      free(vertex_shader_code);
      return 0;
   }

   // Compile shaders.
   glShaderSource(vertex_shader_id, 1, &vertex_shader_code, 0);
   glCompileShader(vertex_shader_id);

   GLint vertex_shader_compile_status;
   int info_log_length;
   glGetShaderiv(vertex_shader_id, GL_COMPILE_STATUS, &vertex_shader_compile_status);
   glGetShaderiv(vertex_shader_id, GL_INFO_LOG_LENGTH, &info_log_length);

   glShaderSource(fragment_shader_id, 1, &fragment_shader_code, 0);
   glCompileShader(fragment_shader_id);


   free(vertex_shader_code);
   free(fragment_shader_code);

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}



	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}



	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}


	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}
