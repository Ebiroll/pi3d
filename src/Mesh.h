#pragma once

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "assimp/scene.h"
#include "assimp/mesh.h"

#include <vector>


struct Texture {
    GLuint id;
    std::string type;
    aiString path;  // We store the path of the texture to compare with other textures
};

GLint TextureFromFile(const char* path, std::string directory, bool gamma);


class Mesh
{
public :
	struct MeshEntry {
        enum BUFFERS {
			VERTEX_BUFFER, TEXCOORD_BUFFER, NORMAL_BUFFER, INDEX_BUFFER
		};
		GLuint vao;
		GLuint vbo[4];

		unsigned int elementCount;

        std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
        MeshEntry(aiMesh *mesh,const aiScene *scene);
		~MeshEntry();

		void load(aiMesh *mesh);
		void render(GLuint prog);
	};

	std::vector<MeshEntry*> meshEntries;

    static std::vector<Texture> textures_loaded;

public:
	Mesh(const char *filename);
	~Mesh(void);

	void render(GLuint prog);
};

