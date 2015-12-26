#include <vector>
#include <map>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include "hash.h"

#include "pkg.h"

using std::uint32_t;


bool printAll=false;

//std::vector<unsigned int> m_indices;
//std::vector<float>        m_vertices;
//std::vector<float>        m_normals;

typedef struct mesh {
  std::vector<unsigned short int>  indexes;
  std::vector<Vertex_t>            vrtx;
  int material;
} t_mesh;

struct Node
{
    std::string             name;
    aiMatrix4x4            _transformation;
    std::vector<int>       _meshes;
    std::vector<Node*>     _nodes;
};


std::vector<t_mesh>     m_meshes;


void writeToMdlFile(char *filename , std::vector<Vertex_t> data,std::vector<unsigned short int> indexes,bool skinned,uint32_t hash)
{

    FILE *ofile = fopen(filename, "wb");

    if (ofile==NULL)
    {
        printf("Failed open file %s for writing ",filename);
        exit(1);
    }


  mdl_lod1Header_t header;

  header.lod[0]=1;
  header.lod[1]=0;
  header.lod[2]=0;
  header.lod[3]=0;

  //header.nlods=1;

    // Find max/min ...
    Vertex_t max;
    Vertex_t min;

    for(int i=0;i<3;i++)
    {
        min.pos[i]=data[0].pos[i];
        max.pos[i]=data[0].pos[i];
    }


    for (int j=0;j<data.size();j++)
    {
        if (printAll) printf("v %f,%f,%f\n",data[j].pos[0],data[j].pos[1],data[j].pos[2]);
        if (printAll) printf("n %f,%f,%f\n",data[j].norm[0],data[j].norm[1],data[j].norm[2]);
        if (printAll) printf("t %f,%f\n",data[j].tex[0],data[j].tex[1]);

        for(int ii=0;ii<3;ii++)
        {
            if (data[j].pos[ii]>max.pos[ii])
            {
                max.pos[ii]=data[j].pos[ii];
            }
            if (data[j].pos[ii]<min.pos[ii])
            {
                min.pos[ii]=data[j].pos[ii];
            }
         }
    }

    header._abb[0].x=min.pos[0];
    header._abb[0].y=min.pos[1];
    header._abb[0].z=min.pos[2];

    header._abb[1].x=max.pos[0];
    header._abb[1].y=max.pos[1];
    header._abb[1].z=max.pos[2];


 if (skinned==false)
 {

     strcpy(header.name,"Test");
     Hash_key key("SIMPLE");
     header.render_hash=key;

     header.texture_hash=hash;
     printf("Hash %u\n",hash);


     fwrite(&header,sizeof(mdl_lod1Header_t),1,ofile);

     uint32_t buffer_size=sizeof(Vertex_t) * data.size();
     fwrite(&buffer_size,sizeof(uint32_t),1,ofile);
     printf("Buffer size %d , %d vertices\n",buffer_size,data.size());

     fwrite(&data[0],1,buffer_size,ofile);

     uint32_t index_size=indexes.size()*2;

     printf("Total %d indexes %d triangles\n",index_size/2,index_size/6);
     fwrite(&index_size,sizeof(uint32_t),1,ofile);

     fwrite(&indexes[0],1,index_size,ofile);

 }
 else
 {

     std::vector<Skinned_Vertex_t> tmpVrtx;

     for(int j=0;j<data.size();j++)
     {
         Skinned_Vertex_t tmp;
         tmp.index=1;
         for(int ii=0;ii<3;ii++)
         {
             tmp.norm[ii]=data[j].norm[ii];
             tmp.pos[ii]=data[j].pos[ii];
         //tmp.
         }
         tmp.tex[0]=data[j].tex[0];
         tmp.tex[1]=data[j].tex[1];
     }

     strcpy(header.name,"Test");
     Hash_key key("BUILDING");
     header.render_hash=key;

     header.texture_hash=hash;
     printf("Hash %u\n",hash);

     fwrite(&header,sizeof(mdl_lod1Header_t),1,ofile);

     uint32_t buffer_size=sizeof(Vertex_t) * tmpVrtx.size();
     fwrite(&buffer_size,sizeof(uint32_t),1,ofile);


     fwrite(&tmpVrtx[0],sizeof(Vertex_t),tmpVrtx.size(),ofile);

     uint32_t index_size=indexes.size()*2;
     fwrite(&index_size,sizeof(uint32_t),1,ofile);


     fwrite(&indexes[0],sizeof(unsigned short int),indexes.size(),ofile);


  }

  fclose(ofile);

}
void processNode(const aiScene *scene, aiNode *node, Node *parentNode, Node &newNode)
{
    newNode.name = node->mName.length != 0 ? node->mName.C_Str() : std::string("None") ;
    newNode._transformation = node->mTransformation;


    for (uint imesh = 0; imesh < node->mNumMeshes; ++imesh)
    {            
            newNode._meshes.push_back(node->mMeshes[imesh]);
    }

    for (uint ich = 0; ich < node->mNumChildren; ++ich)
    {
        newNode._nodes.push_back(new Node());
        processNode(scene, node->mChildren[ich], parentNode, *(newNode._nodes[ich]));
    }
}

/*

struct Node
{
    std::string name;
    aiMatrix4x4            _transformation;
    std::vector<t_mesh>    _meshes;
    std::vector<Node*>     _nodes;
};

typedef struct Vertex
{
        float pos[3];
        float norm[3];
        float tex[2];
} Vertex_t;


*/

std::vector<Vertex_t> allUsedMeshes;

std::vector<unsigned short int> allUsedIndexes;

void mergeAllNodes(Node *node,const aiScene* scene)
{

    for (uint i = 0;i <node->_meshes.size();i++)
    {
        for (int j=0;j<scene->mMeshes[node->_meshes[i]]->mNumVertices;j++)
        {
           aiMesh* mesh=scene->mMeshes[node->_meshes[i]];

           aiVector3D &vec = mesh->mVertices[j];
           aiVector3t<float>  mypos(vec.x,vec.y,vec.z);

           aiMatrix4x4 trans=node->_transformation;
           aiVector3t<float> result=trans*mypos;
           //aiVector3t<float> result=mypos;


           {
               aiVector3D &norm = mesh->mNormals[j];
               aiVector3D tex = mesh->mTextureCoords[0][j];
               unsigned int numUV=mesh->mNumUVComponents[j];
               Vertex_t    tmp;

               tmp.pos[0]=result.x;
               tmp.pos[1]=result.y;
               tmp.pos[2]=result.z;

               tmp.norm[0]=norm.x;
               tmp.norm[1]=norm.y;
               tmp.norm[2]=norm.z;

               tmp.tex[0]=tex.x;
               tmp.tex[1]=tex.y;

               allUsedIndexes.push_back(allUsedMeshes.size());
               allUsedMeshes.push_back(tmp);

          }


           //printf("[%f,%f,%f]\n",result.x,result.y,result.z);

           if (!trans.IsIdentity()) {
               printf("WARNING NODE Transformations are not handled so well by ass2mdl!!!\n");
               printf("[%f,%f,%f,%f\n",trans.a1,trans.a2,trans.a3,trans.a4);
               printf("%f,%f,%f,%f\n",trans.b1,trans.b2,trans.b3,trans.b4);
               printf("%f,%f,%f,%f\n",trans.c1,trans.c2,trans.c3,trans.c4);
               printf("%f,%f,%f,%f]\n",trans.d1,trans.d2,trans.d3,trans.d4);

           }
        }
    }

    for (uint ich = 0; ich < node->_nodes.size(); ++ich)
    {
        aiMatrix4x4 trans=node->_transformation;
        aiMatrix4x4 child=node->_nodes[ich]->_transformation;

        aiMatrix4x4 result=trans*child;
        node->_nodes[ich]->_transformation=result;


        mergeAllNodes(node->_nodes[ich],scene);
    }

}




bool processMesh(aiMesh *mesh,std::vector<Vertex_t> &ret,std::vector<unsigned short int> &indexes,int &material,bool skinned)
{

    if (mesh->mName.length != 0)
    {
        printf("Mesh name, %s\n",mesh->mName.C_Str());
    }

    // Get Vertices
    if (mesh->mNumVertices > 0)
    {
        printf ("Has %d vertices\n",mesh->mNumVertices);
        printf ("Has %d faces\n",mesh->mNumFaces);
        Vertex_t    tmp;

        for (uint t = 0; t < mesh->mNumVertices; t++)
        {
                aiVector3D &vec = mesh->mVertices[t];
                aiVector3D &norm = mesh->mNormals[t];
                aiVector3D tex = mesh->mTextureCoords[0][t];
                unsigned int numUV=mesh->mNumUVComponents[t];

                tmp.pos[0]=mesh->mVertices[t].x;
                tmp.pos[1]=mesh->mVertices[t].y;
                tmp.pos[2]=mesh->mVertices[t].z;

                tmp.norm[0]=norm.x;
                tmp.norm[1]=norm.y;
                tmp.norm[2]=norm.z;

                tmp.tex[0]=tex.x;
                tmp.tex[1]=tex.y;

                indexes.push_back(ret.size());
                ret.push_back(tmp);

            //printf("---%d",face->mIndices[0]);
        }

    }



 #if 0
        for (uint t = 0; t < mesh->mNumFaces; ++t)
        {
            aiFace* face = &mesh->mFaces[t];
            if (face->mNumIndices != 3)
            {
                printf( "Warning: Mesh face with not exactly 3 indices, ignoring this primitive.");
                continue;
            }

            for(unsigned int ii=0;ii<3;ii++)
            {
                aiVector3D &vec = mesh->mVertices[face->mIndices[ii]];
                aiVector3D &norm = mesh->mNormals[face->mIndices[ii]];
                aiVector3D tex = mesh->mTextureCoords[0][face->mIndices[ii]];
                unsigned int numUV=mesh->mNumUVComponents[face->mIndices[ii]];
                Vertex_t    tmp;

                tmp.pos[0]=vec.x;
                tmp.pos[1]=vec.y;
                tmp.pos[2]=vec.z;

                tmp.norm[0]=norm.x;
                tmp.norm[1]=norm.y;
                tmp.norm[2]=norm.z;

                tmp.tex[0]=tex.x;
                tmp.tex[1]=tex.y;

                indexes.push_back(ret.size());
                ret.push_back(tmp);
            }

            //printf("---%d",face->mIndices[0]);
        }

    }
#endif

    printf("%d indexes\n",indexes.size());

    //newMesh->indexCount = m_indices.size() - indexCountBefore;
    //newMesh->material = m_materials.at(mesh->mMaterialIndex);
    material=mesh->mMaterialIndex;

    return true;
}


void printHelp(int argc, char *argv[]) {


    printf("Usage, %s [-t filename] [-m meshnum] -i mtl_ix   [-h] [-o outname] modelname.type\n",argv[0]);

    printf("     -o   output name defaults to modelname.pkg\n");
    printf("     -d   Print all vertices\n");
    printf("     -t   texture filename to create hash with.\n");
    printf("     -m   What mesh to extract.\n");
    printf("     -a   Extract all meshes.\n");
    printf("     -i   material index.\n");
    printf("     -s   Skinned rendertype\n");
    printf("     -h   This helptext\n");

    // -d   Displays all header metadata
    // -of  The old box mdl example format, without render_type
}


int main (int argc, char *argv[], char **env_var_ptr)
{
int i=0;
int mtlIndex=0;
char out_filename[256];
char mdl_filename[256];
bool skinned=false;
bool all=false;

uint32_t hash=0;
/*
    vis::Hash_key key("SIMPLE");
    hash=key;
    printf("SIMPLE %u\n",hash);

    // Skinned types.....

    vis::Hash_key key("BUILDING");
    hash=key;
    printf("BUILDING %u\n",hash);


    // AV_MODEL
    // AV_CS
    // STATIC_DECAL
*/


    char *filename=argv[argc-1];
    int    extractMeshnum=-1;

    strcpy(out_filename,filename);


    for (i=1; i< argc; i++) {
      if (!strcmp(argv[i],"-h"))
      {
          printHelp(argc,argv);
          exit(1);
      }

      if (!strcmp(argv[i],"-t"))
      {
          Hash_key key(argv[i+1]);
          hash=key;
      }

      if (!strcmp(argv[i],"-d"))
      {
          printAll=true;
      }

      if (!strcmp(argv[i],"-s"))
      {
          skinned=true;
      }

      if (!strcmp(argv[i],"-a"))
      {
          all=true;
      }


      if (!strcmp(argv[i],"-m"))
      {
          extractMeshnum=atoi(argv[i+1]);
      }

      if (!strcmp(argv[i],"-i"))
      {
          mtlIndex=atoi(argv[i+1]);
      }


      if (!strcmp(argv[i],"-o"))
      {
        strcpy(out_filename,argv[i+1]);
      }

      // printf("\narg%d=%s", i, argv[i]);
     }


    // Extract base
    char *pExt = strrchr(out_filename, '.');
    if (pExt != NULL)
        strcpy(pExt, ".mdl");
    else
        strcat(out_filename, ".mdl");

    Assimp::Importer importer;

    //const aiScene* scene = importer.ReadFile(filename,NULL);
    //             aiProcess_GenSmoothNormals |
    //          |  aiProcess_SortByPType

    const aiScene* scene = importer.ReadFile(filename,
            aiProcess_RemoveRedundantMaterials |
            aiProcess_Triangulate | aiProcess_GenSmoothNormals
            );

    if (scene==NULL)
    {
        std::string tmp;
        printf("Not able to read file error:%s\n",importer.GetErrorString());



        importer.GetExtensionList(tmp);
        printf("Supported extensions %s \n",tmp.c_str());
        exit(-1);

    }

    printf ("Found %d meshes\n",scene->mNumMeshes);
    printf ("Found %d materials\n",scene->mNumMaterials);
    printf ("Found %d animations\n",scene->mNumAnimations);


    std::vector<unsigned short int> indexTotal;
    std::vector<Vertex_t>           vrtxTotal;

    if (scene->HasMeshes())
        {
            for (unsigned int ii = 0; ii < scene->mNumMeshes; ++ii)
            {
                std::vector<unsigned short int> indexes;
                t_mesh   tmp;
                tmp.indexes.clear();
                tmp.vrtx.clear();
                tmp.material=0;

                processMesh(scene->mMeshes[ii],tmp.vrtx,tmp.indexes,tmp.material,skinned);
                m_meshes.push_back(tmp);


                int offset=vrtxTotal.size();
                for(int q=0;q<tmp.vrtx.size();q++)
                {
                    vrtxTotal.push_back(tmp.vrtx[q]);
                }
                for(int q=0;q<tmp.indexes.size();q++)
                {
                    indexTotal.push_back(offset+tmp.indexes[q]);
                    int qk=offset+tmp.indexes[q];
                    printf("index %d ",offset+tmp.indexes[q]);
                    printf("(%f,%f,%f)\n",vrtxTotal[qk].pos[0],vrtxTotal[qk].pos[1],vrtxTotal[qk].pos[2]);
                }

                printf("Mesh %d Has %d vertices and material %d\n",ii,scene->mMeshes[ii]->mNumVertices,tmp.material);
                //scene->mMaterials[material]->GetTexture()
                if (ii==extractMeshnum )
                {
                    char Buff[512];
                    sprintf(Buff,"%d-%s",ii,out_filename);
                    writeToMdlFile(Buff,tmp.vrtx,tmp.indexes,skinned,hash);
                }

            }

            writeToMdlFile(out_filename,vrtxTotal,indexTotal,skinned,hash);


#if 0

            // All meshes are cached, extract nodes
            if (scene->mRootNode != NULL)
            {
                Node *rootNode = new Node;
                processNode(scene, scene->mRootNode, 0, *rootNode);

                allUsedMeshes.clear();
                allUsedIndexes.clear();

                mergeAllNodes(rootNode,scene);

                writeToMdlFile(out_filename,allUsedMeshes,allUsedIndexes,skinned,hash);
                //m_rootNode.reset(rootNode);


            }
            else
            {
                printf("Error loading model, No ROOT node");
                exit(-1);
            }
#endif



        }
        else
        {
            printf("Error: No meshes found");
            return false;
        }

    for (unsigned int ii = 0; ii < scene->mNumMaterials; ++ii)
    {
        //printf("Material %d has %d textures\n",ii,scene->mMaterials[ii]->GetTextureCount());
                //QSharedPointer<MaterialInfo> mater = processMaterial(scene->mMaterials[ii]);
                //m_materials.push_back(mater);
    }



}


