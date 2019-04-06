#pragma once
#ifndef MODEL_H
#define MODEL_H

#include <QOpenGLFunctions_4_4_Core>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <stb_image_write.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


#include "mesh2.h"

#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;

//unsigned int newTextureFromFile(const char *path, const string &directory, bool gamma = false);
const QVector3D v0( -0.5, -0.5, -0.5 );
const QVector3D v1(0.5, -0.5, -0.5);
const QVector3D v2(0.5, 0.5, -0.5);
const QVector3D v3(-0.5, 0.5, -0.5);

const QVector3D v4(-0.5, -0.5, 0.5);
const QVector3D v5(0.5, -0.5, 0.5);
const QVector3D v6(0.5, 0.5, 0.5);
const QVector3D v7(-0.5, 0.5, 0.5);


class Model
{
public:
    /*  Model Data */
    vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    vector<Mesh2> meshes;
    string directory;
    bool gammaCorrection;

    /*  Functions   */
    // constructor, expects a filepath to a 3D model.
    Model(){

    }

    Model(string const &path, bool gamma = false) : gammaCorrection(gamma)
    {
        meshCounter=0;
        loadModel(path);
    }
    vector<Mesh2> getMeshes(){
        return meshes;
    }
    // draws the model, and thus all its meshes
    void Draw(QOpenGLShaderProgram* m_shader)
    {
        for (unsigned int i = 0; i < meshes.size(); i++){

            meshes[i].Draw(m_shader);
        }
    }

	void getFacePoints(unsigned int i, int(&res)[3]){
		if (i != 0)
		{
			res[0] = i-1;
			res[1] = i;
			res[2] = i+1;
		}
		
	}

	void getMinMaxTriangle(vector<glm::ivec2> points, int& xmin, int& xmax, int& ymin, int& ymax){
		for (int i = 0; i < 3; i++){
			if (points[i].x < xmin)
				xmin = points[i].x;
			if (points[i].x > xmax)
				xmax = points[i].x;

			if (points[i].y < ymin)
				ymin = points[i].y;
			if (points[i].y > ymax)
				ymax = points[i].y;
		}
		/*qWarning() <<  xmin;
		qWarning() <<  xmax;
		qWarning() <<  ymin;
		qWarning() <<  ymax;*/
		
	}

	void convertToPixels(vector<glm::vec2> pointPos, vector<glm::ivec2>(&pixelPos), int w, int h){
		glm::vec2 imSize;
		imSize.x = (double)w;
		imSize.y = (double)h;
		for (int i = 0; i < 3; i++){
			pixelPos.push_back(floor(pointPos[i] * imSize));
		}
	}

	float area(int x1, int y1, int x2, int y2, int x3, int y3)
	{
		return abs((x1*(y2 - y3) + x2*(y3 - y1) + x3*(y1 - y2)) / 2.0);
	}

	bool isInside(int x1, int y1, int x2, int y2, int x3, int y3, int x, int y, int w, int h)
	{
		float threshhold = (float)(w / 500.0);
		float t1 = sqrt((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1));
		float t2 = sqrt((x2 - x3)*(x2 - x3) + (y2 - y3)*(y2 - y3));
		float t3 = sqrt((x3 - x1)*(x3 - x1) + (y3 - y1)*(y3 - y1));
		//qWarning() << t1 << t2 << t3 << threshhold;
		
		//if ( t1 > threshhold ||
		//	 t2 > threshhold ||
		//	 t3 > threshhold){
		//	//qWarning() << "Found";
		//	return	true;
		//}/* Calculate area of triangle ABC */
		double A = area(x1, y1, x2, y2, x3, y3);

		/* Calculate area of triangle PBC */
		double A1 = area(x, y, x2, y2, x3, y3);

		/* Calculate area of triangle PAC */
		double A2 = area(x1, y1, x, y, x3, y3);

		/* Calculate area of triangle PAB */
		double A3 = area(x1, y1, x2, y2, x, y);
		
		/* Check if sum of A1, A2 and A3 is same as A */
		return (A < A1 + A2 + A3 + 0.006 && A > A1 + A2 + A3 - 0.006);
	}

	bool isInsideCube(float x, float y, float z, float minX, float maxX, float minY, float maxY, float minZ, float maxZ){
		if (x > minX && x < maxX){
			if (y > minY && y < maxY){
				if (z > minZ && z < maxZ){
					return true;
				}
			}
		}
		return false;
	}

	void export3DModel(const QMatrix4x4 model, const QMatrix4x4 objModel, QString dir){
		vector<Texture> texturesToExport = textures_loaded;
		vector<QImage> images;
		int count = 0;
		for (int i = 0; i < texturesToExport.size(); i++){
			
				images.push_back(QImage());
				QFile f(texturesToExport[i].path.data);
				QFileInfo fi(f);
				QString qExt = fi.suffix();
				qWarning() << qExt;
				qExt = qExt.toUpper();
				qWarning() << (directory + '/' + texturesToExport[i].path.data).c_str();
				const char * ext = qExt.toUtf8();
				qWarning() << images[count].load((directory + '/' + texturesToExport[i].path.data).c_str(), ext) << "image load";
				count++;
			
		}
		QVector3D v0up = model*v0;
		QVector3D v1up = model*v1;
		QVector3D v2up = model*v2;
		QVector3D v3up = model*v3;
		QVector3D v4up = model*v4;
		QVector3D v5up = model*v5;
		QVector3D v6up = model*v6;
		QVector3D v7up = model*v7;
		vector<QVector3D> points = { v0up, v1up, v2up, v3up, v4up, v5up, v6up, v7up };
		
		float minX = 1000000.0f; float minY = 1000000.0f; float minZ = 1000000.0f;
		float maxX = -1000000.0f; float maxY = -1000000.0f; float maxZ = -1000000.0f;
		for (int k = 0; k < 8; k++){
			if (points[k].x() < minX)
				minX = points[k].x();
			if (points[k].x() > maxX)
				maxX = points[k].x();

			if (points[k].y() < minY)
				minY = points[k].y();
			if (points[k].y() > maxY)
				maxY = points[k].y();

			if (points[k].z() < minZ)
				minZ = points[k].z();
			if (points[k].z()> maxZ)
				maxZ = points[k].z();
		}
		QColor col(255, 255, 255, 0);
		int countIndices = 0;
		for (int i = 0; i < meshes.size(); i++){
			for (int k = 0; k < texturesToExport.size(); k++){
				for (int l = 0; l < meshes[i].textures.size(); l++){
					if (meshes[i].textures[l].id == texturesToExport[k].id && texturesToExport[k].type == "texture_diffuse"){
						for (int u = 1; u < meshes[i].indices.size(); u+=3){
							int j = meshes[i].indices[u];
							QVector3D modeled = objModel*QVector3D(meshes[i].vertices[j].Position.x, meshes[i].vertices[j].Position.y, meshes[i].vertices[j].Position.z );
							float x = modeled.x();
							float y = modeled.y();
							float z = modeled.z();
							if (isInsideCube(x, y, z, minX, maxX, minY, maxY, minZ, maxZ)){
								int facePoints[3];
								getFacePoints(meshes[i].indices[u], facePoints);
								/*facePoints[0] -= countIndices;
								facePoints[1] -= countIndices;
								facePoints[2] -= countIndices;*/
								QVector3D modeled2 = objModel*QVector3D(meshes[i].vertices[facePoints[0]].Position.x, meshes[i].vertices[facePoints[0]].Position.y, meshes[i].vertices[facePoints[0]].Position.z);

								float x1 = modeled2.x();
								float y1 = modeled2.y();
								float z1 = modeled2.z();
								
								QVector3D modeled3 = objModel*QVector3D(meshes[i].vertices[facePoints[2]].Position.x, meshes[i].vertices[facePoints[2]].Position.y, meshes[i].vertices[facePoints[2]].Position.z);
								float x2 = modeled3.x();
								float y2 = modeled3.y();
								float z2 = modeled3.z();
								if (isInsideCube(x1, y1, z1, minX, maxX, minY, maxY, minZ, maxZ)
									&& isInsideCube(x2, y2, z2, minX, maxX, minY, maxY, minZ, maxZ)){

									vector<glm::vec2> pointPos = { meshes[i].vertices[facePoints[0]].TexCoords, meshes[i].vertices[facePoints[1]].TexCoords, meshes[i].vertices[facePoints[2]].TexCoords };
									int width = images[k].width();
									int height = images[k].height();
									vector<glm::ivec2> pixelPos;
									convertToPixels(pointPos, pixelPos, width, height);
									int xmin, xmax, ymin, ymax;
									xmin = 1000000;
									xmax = -1;
									ymin = 1000000;
									ymax = -1;
									getMinMaxTriangle(pixelPos, xmin, xmax, ymin, ymax);
									images[k].setPixelColor(pixelPos[0].x, pixelPos[0].y, col);
									images[k].setPixelColor(pixelPos[1].x, pixelPos[1].y, col);
									images[k].setPixelColor(pixelPos[2].x, pixelPos[2].y, col);
									for (int p = xmin-1; p <= xmax+1; p++){
										for (int o = ymin-1; o <= ymax+1; o++){
											if (isInside(pixelPos[0].x, pixelPos[0].y, pixelPos[1].x, pixelPos[1].y, pixelPos[2].x, pixelPos[2].y, p, o, width, height)){
												images[k].setPixelColor(p, o, col);
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		
		qWarning() << meshes[0].vertices.size();
		qWarning() << QDir::currentPath();
		for (int i = 0; i < images.size(); i++){
			qWarning() << dir;
			QString filename(dir+"/test" + QString::number(i) + ".png");
			qWarning() << filename;
			qWarning() << images[i].save(filename, "PNG");
		}
		
	}

private:

    int meshCounter;
    /*  Functions   */
    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(string const &path)
    {
        meshes.clear();
        // read file via ASSIMP
        Assimp::Importer *importer = new Assimp::Importer;

        const aiScene* scene = new const aiScene;

        //const aiScene* scene = importer->ReadFileFromMemory((const void*)ss, size, aiProcess_Triangulate );
        scene = importer->ReadFile(path.c_str(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_GenNormals);
        // check for errors
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            cout << "ERROR::ASSIMP:: " << importer->GetErrorString() << endl;
            return;
        }
        // retrieve the directory path of the filepath
        directory = path.substr(0, path.find_last_of('/'));

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene);
    }

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode *node, const aiScene *scene)
    {
        // process each mesh located at the current node
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            meshCounter++;
            // the node object only contains indices to index the actual objects in the scene.
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene, i));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }

    }

    Mesh2 processMesh(aiMesh *mesh, const aiScene *scene, int MeshIndex)
    {
        // data to fill
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;
        
        float bboxMinX=1000000.0f;float bboxMinY=1000000.0f;float bboxMinZ=1000000.0f;
            float bboxMaxX=-1000000.0f;float bboxMaxY=-1000000.0f;float bboxMaxZ=-1000000.0f;
            for(uint v=0; v < mesh->mNumVertices; ++v){
                float valX=mesh->mVertices[v].x;
                float valY=mesh->mVertices[v].y;
                float valZ=mesh->mVertices[v].z;
                if(valX<bboxMinX)
                    bboxMinX=valX;
                if(valX>bboxMaxX)
                    bboxMaxX=valX;

                if(valY<bboxMinY)
                    bboxMinY=valY;
                if(valY>bboxMaxY)
                    bboxMaxY=valY;

                if(valZ<bboxMinZ)
                    bboxMinZ=valZ;
                if(valZ>bboxMaxZ)
                    bboxMaxZ=valZ;
            }
        
            float sizeX=(bboxMaxX-bboxMinX);
            float sizeY=(bboxMaxY-bboxMinY);
            float sizeZ=(bboxMaxZ-bboxMinZ);
            float maxSize=max(sizeX, max(sizeY, sizeZ));

        // Walk through each of the mesh's vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
                              // positions

            float tmpx = (mesh->mVertices[i].x-bboxMinX)/(maxSize);
            float tmpy = (mesh->mVertices[i].y-bboxMinY)/(maxSize);
            float tmpz = (mesh->mVertices[i].z-bboxMinZ)/(maxSize);
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            // normals
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
            vertex.meshIndx = meshCounter;
            // texture coordinates
            if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                glm::vec2 vec;
                // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            // tangent
//            vector.x = mesh->mTangents[i].x;
//            vector.y = mesh->mTangents[i].y;
//            vector.z = mesh->mTangents[i].z;
//            vertex.Tangent = vector;
//            // bitangent
//            vector.x = mesh->mBitangents[i].x;
//            vector.y = mesh->mBitangents[i].y;
//            vector.z = mesh->mBitangents[i].z;
//            vertex.Bitangent = vector;
            vertices.push_back(vertex);
        }
        // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }
        // process materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
        // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
        // Same applies to other texture as the following list summarizes:
        // diffuse: texture_diffuseN
        // specular: texture_specularN
        // normal: texture_normalN

        // 1. diffuse maps
        vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 2. specular maps
        vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // 3. normal maps
        std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // 4. height maps
        std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_SHININESS, "texture_roughness");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
        Mesh2 *m = new Mesh2(vertices, indices, textures);

        // return a mesh object created from the extracted mesh data
        return *m;
    }



    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName)
    {
        vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if (std::strcmp(textures_loaded[j].path.C_Str(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                    break;
                }
            }
            if (!skip)
            {   // if texture hasn't been loaded already, load it
                Texture texture;
                texture.id = newTextureFromFile(str.C_Str(), this->directory, false);
                texture.type = typeName;
                texture.path = str;
                textures.push_back(texture);
                textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
            }
        }
        return textures;
    }


    unsigned int newTextureFromFile(const char *path, const string &directory, bool gamma)
    {
        string filename = string(path);
        filename = directory + '/' + filename;
		
        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;
		
        unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            GLenum format;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            f->glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


//            unsigned char *data2 = data;
//            stbi_image_free(data);

//            for (int i = 1000; i<1500;i++){
//                for(int j=1000; j<1500; j++){
//                    unsigned char* pixel = data2 + (i + height*j)*nrComponents;
//                    pixel[0] = '255';
//                    pixel[1] = '255';
//                    pixel[2] = '255';
//                    pixel[3] = '0';
//                }
//            }

            //int a = stbi_write_jpg('test.jpg', width, height, nrComponents,data2,70);
        }
        else
        {
            std::cout << "Texture failed to load at path: " << path << std::endl;
            stbi_image_free(data);
        }

        return textureID;
    }

};



#endif
