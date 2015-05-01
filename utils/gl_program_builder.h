#include <GL/glew.h>
#include <errno.h>
#include <string.h>

#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <functional>
#include <iterator>

#ifndef PROGRAM_BUILDER_H_
#define PROGRAM_BUILDER_H_

struct ProgramBuilder {
//  std::vector<std::string> includeDirs;

  static GLuint buildComputeProgramFromString(const std::string shader) {
    GLuint program = glCreateProgram();

    GLuint compute_shader = compile(GL_COMPUTE_SHADER, shader);

    glAttachShader(program, compute_shader);
    glLinkProgram(program);

    if (!logLinkStatus(program))
      std::runtime_error("Failed to link shaders");
    glDeleteShader(compute_shader);

    return program;
  }

  static GLuint buildComputeProgramFromFile(const std::string shader) {
    GLuint program = glCreateProgram();

    GLuint compute_shader = compileFromFile(GL_COMPUTE_SHADER, shader);

    glAttachShader(program, compute_shader);
    glLinkProgram(program);

    if (!logLinkStatus(program))
      std::runtime_error("Failed to link shaders");
    glDeleteShader(compute_shader);

    return program;
  }

  static GLuint buildFromFiles(const std::string& vert, const std::string& frag) {
    GLuint program = glCreateProgram();

    GLuint vert_shader = compileFromFile(GL_VERTEX_SHADER, vert);
    GLuint frag_shader = compileFromFile(GL_FRAGMENT_SHADER, frag);

    glAttachShader(program, vert_shader);
    glAttachShader(program, frag_shader);
    glLinkProgram(program);

    if (!logLinkStatus(program))
      std::runtime_error("Failed to link shaders");
    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);

    return program;
  }

  static GLuint buildFromStrings(const std::string& vert, const std::string& frag) {
    GLuint program = glCreateProgram();
    GLuint vert_shader = compile(GL_VERTEX_SHADER, vert);
    GLuint frag_shader = compile(GL_FRAGMENT_SHADER, frag);
    glAttachShader(program, vert_shader);
    glAttachShader(program, frag_shader);
    glLinkProgram(program);

    logLinkStatus(program);
    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);
    return program;
  }

private:

  static std::string preprocess(const std::string& input) {
    std::string res;
    std::istringstream iss(input);
    for(std::string line; std::getline(iss, line); ) {
      // Copy original line to alter it
      std::string line_cpy = line;

      // Clear whitespace before the line
      line_cpy.erase(line_cpy.begin(),
          std::find_if(line_cpy.begin(), line_cpy.end(), std::ptr_fun<int, int>(std::isgraph)));

      // Check for preprocessor directives
      if(line_cpy.size() > 0 && line_cpy[0] == '#') {
        // Split line into tokens by whitespace
        std::istringstream buf(line_cpy);
        std::vector<std::string> tokens{std::istream_iterator<std::string>(buf),
                                        std::istream_iterator<std::string>()};
        // Handle #pragma directives
        if(tokens[0] == "#pragma") {
          if(tokens[1] == "include") {
            std::string filename = tokens[2].substr(1, tokens[2].size()-2);
            std::string file_str = readFileToString(filename);
            file_str = file_str.append(std::string("\n"));
            res = res.append(file_str);
            continue;
          }
        }
      }
      line.append(std::string("\n"));
      res = res.append(line);
    }
    std::cerr << "LENGTH1: " << res.size() << std::endl;
    return res;
  }

  static std::string readFileToString(const std::string& filename) {
    std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
    if (in) {
      std::string contents;
      in.seekg(0, std::ios::end);
      contents.resize((unsigned int) in.tellg());
      in.seekg(0, std::ios::beg);
      in.read(&contents[0], contents.size());
      in.close();
      return (contents);
    }
    std::ostringstream err;
    err << "Error reading file " << filename << " with ERRNO: " << strerror(errno);
    std::runtime_error(err.str().c_str());
    return "";
  }

  static GLuint compile(const GLenum type, const std::string& src) {
    std::string s = preprocess(src);

    GLuint shader = glCreateShader(type);
    const GLchar* source = s.c_str();

    glShaderSource(shader, 1, &source, nullptr);

    glCompileShader(shader);

    if (logCompileStatus(shader, type) != 0) {
      std::runtime_error("Compilation failed\n");
    }

    return shader;
  }

  static GLuint compileFromFile(const GLenum type, const std::string& file_path) {
    fprintf(stdout, "Reading %s: %s\n", shaderTypeAsString(type).c_str(), file_path.c_str());

    std::string src = readFileToString(file_path);
    return compile(type, src);
  }

  static GLint logCompileStatus(const GLuint shader_id, const GLenum type) {
    GLint success = GL_FALSE;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
    std::string log = getCompileLog(shader_id);
    if (success != GL_TRUE) {
      fprintf(stderr, "Error compiling %s:\n\n", shaderTypeAsString(type).c_str());
      if (log.length() > 1) {
        fprintf(stderr, "Compile log:\n");
        fprintf(stderr, "%s\n", log.c_str());
      }

      return -1;
    } else {
      fprintf(stdout, "Successfully compiled %s\n", shaderTypeAsString(type).c_str());
      if (log.length() > 1) {
        fprintf(stderr, "Compile log:\n");
        fprintf(stderr, "%s\n", log.c_str());
      }
      return 0;
    }
  }

  static std::string getCompileLog(const GLuint shader_id) {
    GLint length;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &length);
    std::string log(length, ' ');
    glGetShaderInfoLog(shader_id, length, &length, &log[0]);
    return log;
  }

  static std::string shaderTypeAsString(const GLenum shader_type) {
    switch (shader_type) {
    case GL_VERTEX_SHADER:
      return std::string("vertex shader");
      break;
    case GL_FRAGMENT_SHADER:
      return std::string("fragment shader");
      break;
    case GL_COMPUTE_SHADER:
      return std::string("compute shader");
      break;
    default:
      return std::string("");
    }
  }

  static bool logLinkStatus(GLuint program_id) {
    GLint success;
    glGetProgramiv(program_id, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
      GLint info_log_length;
      glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &info_log_length);

      GLchar str_info_log[info_log_length + 1];
      glGetProgramInfoLog(program_id, info_log_length, 0, str_info_log);
      fprintf(stderr, "Error linking shader:\n");
      fprintf(stderr, "%s\n", str_info_log);
      return false;
    } else {
      fprintf(stdout, "Successfully linked program\n");
      return true;
    }
  }
};

#endif /* PROGRAM_BUILDER_H_ */
