#include <iostream>
#include <tuple>
#include <unordered_map>
#include <functional>
#include <climits>
#include <type_traits>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>



#include "utils/gl_utils.h"
#include "utils/interactive_window.h"

#include "geometry/tile_mesh.h"

using namespace std;
using namespace glm;
using namespace geometry;
using namespace utils;

class App: public InteractiveGLWindow {
	QuadPlanarTileMapV<GLuint> tileMap;
	GLProgramBuilder programBuilder;

	GLuint solidTextureProgram = 0;
	GLuint solidColorProgram = 0;
	GLuint colorTilesProgram = 0;

	unique_ptr<TileMesh<QuadPlanarTileSet>> tileMesh;

public:
	App(unsigned w, unsigned h) : InteractiveGLWindow(w, h) {}

	void onCreate(Renderer& rndr) {
		check_gl_error();
		rndr.setClearColor(vec4(0.1, 0.1, 0.1, 1.0));
		rndr.enableFaceCulling();
		rndr.enableAlphaBlending();
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		programBuilder.addIncludeDir("shaders/glsl330");
		programBuilder.addIncludeDir("shaders");

		solidTextureProgram = programBuilder.buildFromFiles(
				"shaders/solid_texture_vert.glsl",
				"shaders/solid_texture_frag.glsl");
		solidColorProgram = programBuilder.buildFromFiles(
				"shaders/solid_color_vert.glsl",
				"shaders/solid_color_frag.glsl");

		colorTilesProgram = programBuilder.buildFromFiles("shaders/tile_color_vert.glsl", "shaders/tile_color_frag.glsl");

		tileMesh = make_unique<TileMesh<QuadPlanarTileSet>>(5);
	}

	void onDraw(Renderer& rndr) {
		rndr.clearViewport();
		rndr.startFrame();

		GLuint prog = solidTextureProgram;//colorTilesProgram;
		rndr.setProgram(prog);
		const vec3 campos = camera().getPosition() - vec3(0.5, 0.0, 0.5);
		const float focal_length = -0.5f;
		const float f_minus_zc = focal_length - campos.z;
		mat4 magicmat(f_minus_zc, 0,          campos.x,     -focal_length * campos.x - 0.5,
				0,          f_minus_zc, campos.y,     -focal_length * campos.y - 0.5,
				0,          0,          focal_length, -focal_length,
				0,          0,          1,            -campos.z);
		glUniformMatrix4fv(glGetUniformLocation(prog, "magicmat"),
				1, GL_FALSE, value_ptr(transpose(magicmat)));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, tileMesh->tileTextureArray());
		glUniform1i(glGetUniformLocation(prog, "texid"), 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D_ARRAY, tileMesh->tileDepthTextureArray());
		glUniform1i(glGetUniformLocation(prog, "depthId"), 1);

//		glUniform1ui(glGetUniformLocation(prog, "numMirrorFaces"), numTextures);
		rndr.draw(tileMesh->geometry(), mat4(1.0), PrimitiveType::TRIANGLES);

		//    glLineWidth(3.0);
		//    rndr.setProgram(solidColorProgram);
		//    glUniform4fv(glGetUniformLocation(solidColorProgram, "color"), 1, value_ptr(vec4(0.0, 1.0, 0.0, 1.0)));
		//    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		//    rndr.draw(grid, scale(mat4(1.0), vec3(1.0)), PrimitiveType::TRIANGLES);
		//    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
};

int main(int argc, char** argv) {
	App w(1000, 1000);
	w.mainLoop();
}
