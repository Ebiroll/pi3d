#include "Mesh.h"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include <iostream>
#include <string>
#include <vector>
#include "SOIL.h"

// GL includes
#include "shader.h"
//#include "camera.h"
//#include "model.h"

using std::vector;
using std::string;


std::vector<Texture> Mesh::textures_loaded;

GLint TextureFromFile(const char* path, string directory, bool gamma)
{
     //Generate texture ID and load texture data
    string filename = string(path);
    filename = directory + '/' + filename;
    GLuint textureID;
    glGenTextures(1, &textureID);
    int width,height;
    int numChannels=0;

    unsigned char* image = SOIL_load_image(filename.c_str(), &width, &height, &numChannels, SOIL_LOAD_RGB);
    if (image==NULL)
    {
        printf("Failed to load texture %s",SOIL_last_result());
    }
    // Assign texture to ID
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, gamma ? GL_SRGB : GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Parameters
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    SOIL_free_image_data(image);
    return textureID;
}


std::vector<Texture> Mesh::MeshEntry::loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
{
    vector<Texture> textures;
    for(GLuint i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        GLboolean skip = false;
        for(GLuint j = 0; j < textures_loaded.size(); j++)
        {
            if(textures_loaded[j].path == str)
            {
                textures.push_back(textures_loaded[j]);
                skip = true;
                break;
            }
        }
        if(!skip)
        {   // If texture hasn't been loaded already, load it
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(),".",false);
            texture.type = typeName;
            texture.path = str;
            textures.push_back(texture);
            textures_loaded.push_back(texture);  // Add to loaded textures
        }
    }
    return textures;
}


#if 0
vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
{
    vector<Texture> textures;
    for(GLuint i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        Texture texture;
        texture.id = TextureFromFile(str.C_Str(), this->directory);
        texture.type = typeName;
        texture.path = str;
        textures.push_back(texture);
    }
    return textures;
}
#endif

/**
*	Constructor, loading the specified aiMesh
**/
Mesh::MeshEntry::MeshEntry(aiMesh *mesh,const aiScene *scene) {
	vbo[VERTEX_BUFFER] = NULL;
	vbo[TEXCOORD_BUFFER] = NULL;
	vbo[NORMAL_BUFFER] = NULL;
	vbo[INDEX_BUFFER] = NULL;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	elementCount = mesh->mNumFaces * 3;

	if(mesh->HasPositions()) {
		float *vertices = new float[mesh->mNumVertices * 3];
		for(int i = 0; i < mesh->mNumVertices; ++i) {
			vertices[i * 3] = mesh->mVertices[i].x;
			vertices[i * 3 + 1] = mesh->mVertices[i].y;
			vertices[i * 3 + 2] = mesh->mVertices[i].z;
		}

		glGenBuffers(1, &vbo[VERTEX_BUFFER]);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[VERTEX_BUFFER]);
		glBufferData(GL_ARRAY_BUFFER, 3 * mesh->mNumVertices * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray (0);



		delete vertices;
	}


	if(mesh->HasTextureCoords(0)) {
		float *texCoords = new float[mesh->mNumVertices * 2];
		for(int i = 0; i < mesh->mNumVertices; ++i) {
			texCoords[i * 2] = mesh->mTextureCoords[0][i].x;
			texCoords[i * 2 + 1] = mesh->mTextureCoords[0][i].y;
		}

		glGenBuffers(1, &vbo[TEXCOORD_BUFFER]);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[TEXCOORD_BUFFER]);
		glBufferData(GL_ARRAY_BUFFER, 2 * mesh->mNumVertices * sizeof(GLfloat), texCoords, GL_STATIC_DRAW);


        //Thank you. By the the way I think that the line (in texcoord loading):
        //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        //should look more like that:
        //glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);


        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray (2);

		delete texCoords;
	}


	if(mesh->HasNormals()) {
		float *normals = new float[mesh->mNumVertices * 3];
		for(int i = 0; i < mesh->mNumVertices; ++i) {
			normals[i * 3] = mesh->mNormals[i].x;
			normals[i * 3 + 1] = mesh->mNormals[i].y;
			normals[i * 3 + 2] = mesh->mNormals[i].z;
		}

		glGenBuffers(1, &vbo[NORMAL_BUFFER]);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[NORMAL_BUFFER]);
		glBufferData(GL_ARRAY_BUFFER, 3 * mesh->mNumVertices * sizeof(GLfloat), normals, GL_STATIC_DRAW);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray (1);

		delete normals;
	}
	

	if(mesh->HasFaces()) {
		unsigned int *indices = new unsigned int[mesh->mNumFaces * 3];
		for(int i = 0; i < mesh->mNumFaces; ++i) {
			indices[i * 3] = mesh->mFaces[i].mIndices[0];
			indices[i * 3 + 1] = mesh->mFaces[i].mIndices[1];
			indices[i * 3 + 2] = mesh->mFaces[i].mIndices[2];
		}

		glGenBuffers(1, &vbo[INDEX_BUFFER]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[INDEX_BUFFER]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * mesh->mNumFaces * sizeof(GLuint), indices, GL_STATIC_DRAW);

		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray (3);

		delete indices;
	}
	

    if(mesh->mMaterialIndex  >= 0)
    {

        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        vector<Texture> diffuseMaps = this->loadMaterialTextures(material,
                                            aiTextureType_DIFFUSE, "texture_diffuse");
        textures_loaded.insert(textures_loaded.end(), diffuseMaps.begin(), diffuseMaps.end());
        vector<Texture> specularMaps = this->loadMaterialTextures(material,
                                            aiTextureType_SPECULAR, "texture_specular");
        textures_loaded.insert(textures_loaded.end(), specularMaps.begin(), specularMaps.end());

    }

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

/**
*	Deletes the allocated OpenGL buffers
**/
Mesh::MeshEntry::~MeshEntry() {
	if(vbo[VERTEX_BUFFER]) {
		glDeleteBuffers(1, &vbo[VERTEX_BUFFER]);
	}

	if(vbo[TEXCOORD_BUFFER]) {
		glDeleteBuffers(1, &vbo[TEXCOORD_BUFFER]);
	}

	if(vbo[NORMAL_BUFFER]) {
		glDeleteBuffers(1, &vbo[NORMAL_BUFFER]);
	}

	if(vbo[INDEX_BUFFER]) {
		glDeleteBuffers(1, &vbo[INDEX_BUFFER]);
	}

	glDeleteVertexArrays(1, &vao);
}

/**
*	Renders this MeshEntry
**/
void Mesh::MeshEntry::render(GLuint prog) {


       if (textures_loaded.size()>0) {
	 printf("%d",textures_loaded[0].id);
	  glActiveTexture(GL_TEXTURE0);
          glBindTexture(GL_TEXTURE_2D, textures_loaded[0].id);
          glUniform1i(glGetUniformLocation(prog, "tex"), 0);	   
       }
       
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, elementCount, GL_UNSIGNED_INT, NULL);
	glBindVertexArray(0);
}

/**
*	Mesh constructor, loads the specified filename if supported by Assimp
**/
Mesh::Mesh(const char *filename)
{
	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(filename, NULL);
	if(!scene) {
		printf("Unable to load mesh: %s\n", importer.GetErrorString());
                exit(-1);
	}

	for(int i = 0; i < scene->mNumMeshes; ++i) {
           meshEntries.push_back(new Mesh::MeshEntry(scene->mMeshes[i],scene));
	}
}

/**
*	Clears all loaded MeshEntries
**/
Mesh::~Mesh(void)
{
	for(int i = 0; i < meshEntries.size(); ++i) {
		delete meshEntries.at(i);
	}
	meshEntries.clear();
}

/**
*	Renders all loaded MeshEntries
**/
void Mesh::render(GLuint prog) {
	for(int i = 0; i < meshEntries.size(); ++i) {
		meshEntries.at(i)->render(prog);
	}
}

