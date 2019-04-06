#ifndef MESH2_H
#define MESH2_H

#include <QOpenGLFunctions_4_4_Core>

#include <assimp/Importer.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
using namespace std;

struct Vertex {
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;

    int meshIndx;
    // texCoords
    glm::vec2 TexCoords;
    // tangent
    glm::vec3 Tangent;
    // bitangent
    glm::vec3 Bitangent;
};

struct Texture {
    unsigned int id;
    string type;
    aiString path;
};

class Mesh2 {

public:

    /*  Mesh Data  */
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;
    unsigned int VAO;
    QOpenGLVertexArrayObject *m_vao;
    QOpenGLBuffer *m_vbo, *m_ebo;


    /*  Functions  */
    // constructor
    Mesh2(){

    }

    Mesh2(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures)
    {


        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;
        // now that we have all the required data, set the vertex buffers and its attribute pointers.
        setupMesh();
    }

    // render the mesh
    void Draw(QOpenGLShaderProgram* shader)
    {
        QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

        QOpenGLFunctions_4_4_Core *f2 = new QOpenGLFunctions_4_4_Core();
        f2->initializeOpenGLFunctions();


        f->glEnableVertexAttribArray(0);
        // bind appropriate textures
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int heightNr = 1;
        for (unsigned int i = 0; i < textures.size(); i++)
        {
            f->glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
            // retrieve texture number (the N in diffuse_textureN)
            stringstream ss;
            string number;
            string name = textures[i].type;
            if (name == "texture_diffuse")
                ss << diffuseNr++; // transfer unsigned int to stream
            else if (name == "texture_specular")
                ss << specularNr++; // transfer unsigned int to stream
            else if (name == "texture_normal")
                ss << normalNr++; // transfer unsigned int to stream
            else if (name == "texture_height")
                ss << heightNr++; // transfer unsigned int to stream
            number = ss.str();
            // now set the sampler to the correct texture unit
            f->glUniform1i(f->glGetUniformLocation(shader->programId(), (name + number).c_str()), i);
            //cout << (name + number).c_str() << endl;
            // and finally bind the texture
            f->glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }

        f2->glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
        f2->glBindVertexArray(0);

        // always good practice to set everything back to defaults once configured.
        f->glActiveTexture(GL_TEXTURE0);
    }
   

private:
    /*  Render data  */
    unsigned int VBO, EBO;

    /*  Functions    */
    // initializes all the buffer objects/arrays
	void setupMesh()
	{
		QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
		f->initializeOpenGLFunctions();
		QOpenGLFunctions_4_4_Core *f2 = new QOpenGLFunctions_4_4_Core();
		f2->initializeOpenGLFunctions();

		// create buffers/arrays
		f2->glGenVertexArrays(1, &VAO);
		f->glGenBuffers(1, &VBO);
		f->glGenBuffers(1, &EBO);

		f2->glBindVertexArray(VAO);
		// load data into vertex buffers
		f->glBindBuffer(GL_ARRAY_BUFFER, VBO);
		// A great thing about structs is that their memory layout is sequential for all its items.
		// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
		// again translates to 3/2 floats which translates to a byte array.
		f->glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

		f->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		f->glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);


		// set the vertex attribute pointers
		// vertex Positions

		f->glEnableVertexAttribArray(0);
		f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		// vertex normals
		f->glEnableVertexAttribArray(1);
		f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
		// vertex texture coords
		f->glEnableVertexAttribArray(2);
		f->glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

		f->glEnableVertexAttribArray(3);
		f->glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, meshIndx));
		// vertex tangent
		f->glEnableVertexAttribArray(4);
		f->glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
		// vertex bitangent
		f->glEnableVertexAttribArray(5);
		f->glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
		//vertex Color


		f2->glBindVertexArray(0);
	}
};
#endif
