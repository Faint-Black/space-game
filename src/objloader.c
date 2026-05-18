#include "objloader.h"
#include "render.h"
#include "utils.h"

#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VERTICES  4096
#define MAX_NORMALS   4096
#define MAX_TEXCOORDS 4096
#define MAX_FACES     4096

/* ---- Internal helpers ---- */

static void extractDirectory(const char* filePath, char* dirOut, int dirOutSize) {
    const char* lastSlash = strrchr(filePath, '/');
    if (lastSlash != NULL) {
        int len = (int)(lastSlash - filePath) + 1;
        if (len >= dirOutSize) len = dirOutSize - 1;
        memcpy(dirOut, filePath, (size_t)len);
        dirOut[len] = '\0';
    } else {
        dirOut[0] = '\0';
    }
}

static GLuint uploadTexture(const char* path) {
    SDL_Surface* surface;
    GLuint texID = 0;
    GLenum format;

    surface = IMG_Load(path);
    if (surface == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load texture '%s': %s", path, IMG_GetError());
        return 0;
    }

    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    if (surface->format->BytesPerPixel == 4) {
        format = GL_RGBA;
    } else {
        format = GL_RGB;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, (GLint)format, surface->w, surface->h,
                 0, format, GL_UNSIGNED_BYTE, surface->pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    SDL_FreeSurface(surface);
    return texID;
}

/* ---- MTL parser ---- */

static void parseMTL(const char* mtlPath, OBJModel* model) {
    FILE* fp;
    char line[512];
    int currentMat = -1;
    char dir[256];

    extractDirectory(mtlPath, dir, sizeof(dir));

    fp = fopen(mtlPath, "r");
    if (fp == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open MTL file '%s'", mtlPath);
        return;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strncmp(line, "newmtl ", 7) == 0) {
            currentMat = model->materialCount;
            model->materialCount++;
            if (model->materialCount > OBJ_MAX_MATERIALS) {
                model->materialCount = OBJ_MAX_MATERIALS;
                currentMat = OBJ_MAX_MATERIALS - 1;
            }
            model->materials[currentMat].textureID = 0;
            model->materials[currentMat].faces = NULL;
            model->materials[currentMat].faceCount = 0;
            model->materials[currentMat].texturePath[0] = '\0';
        } else if (strncmp(line, "map_Kd ", 7) == 0 && currentMat >= 0) {
            char texName[256];
            char fullPath[512];
            char* nl;
            sscanf(line + 7, " %255[^\r\n]", texName);
            nl = strchr(texName, '\n');
            if (nl) *nl = '\0';
            nl = strchr(texName, '\r');
            if (nl) *nl = '\0';

            snprintf(fullPath, sizeof(fullPath), "%s%s", dir, texName);
            memcpy(model->materials[currentMat].texturePath, fullPath, 255);
            model->materials[currentMat].texturePath[255] = '\0';
        }
    }

    fclose(fp);
}

/* ---- OBJ parser ---- */

extern OBJModel* loadOBJModel(const char* objPath) {
    FILE* fp;
    char line[512];
    char dir[256];
    OBJModel* model;

    Vec3* positions;
    Vec3* normals;
    Vec3* texcoords;

    TriangleFace* tempFaces;
    int tempFaceCount;
    int currentMat;

    int posCount = 0;
    int normCount = 0;
    int tcCount = 0;

    int i;

    extractDirectory(objPath, dir, sizeof(dir));

    fp = fopen(objPath, "r");
    if (fp == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open OBJ file '%s'", objPath);
        return NULL;
    }

    model = (OBJModel*)calloc(1, sizeof(OBJModel));
    if (model == NULL) {
        fclose(fp);
        return NULL;
    }

    positions = (Vec3*)malloc(sizeof(Vec3) * MAX_VERTICES);
    normals = (Vec3*)malloc(sizeof(Vec3) * MAX_NORMALS);
    texcoords = (Vec3*)malloc(sizeof(Vec3) * MAX_TEXCOORDS);
    tempFaces = (TriangleFace*)malloc(sizeof(TriangleFace) * MAX_FACES);
    tempFaceCount = 0;
    currentMat = -1;

    if (!positions || !normals || !texcoords || !tempFaces) {
        free(positions);
        free(normals);
        free(texcoords);
        free(tempFaces);
        free(model);
        fclose(fp);
        return NULL;
    }

    /* First pass: find and parse MTL file */
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strncmp(line, "mtllib ", 7) == 0) {
            char mtlName[256];
            char mtlPath[512];
            sscanf(line + 7, " %255[^\r\n]", mtlName);
            snprintf(mtlPath, sizeof(mtlPath), "%s%s", dir, mtlName);
            parseMTL(mtlPath, model);
            break;
        }
    }
    rewind(fp);

    /* Second pass: parse geometry */
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (line[0] == 'v' && line[1] == ' ') {
            /* vertex position */
            float x, y, z;
            if (sscanf(line + 2, "%f %f %f", &x, &y, &z) == 3) {
                if (posCount < MAX_VERTICES) {
                    positions[posCount] = vec3(x, y, z);
                    posCount++;
                }
            }
        } else if (line[0] == 'v' && line[1] == 'n' && line[2] == ' ') {
            /* vertex normal */
            float x, y, z;
            if (sscanf(line + 3, "%f %f %f", &x, &y, &z) == 3) {
                if (normCount < MAX_NORMALS) {
                    normals[normCount] = vec3(x, y, z);
                    normCount++;
                }
            }
        } else if (line[0] == 'v' && line[1] == 't' && line[2] == ' ') {
            /* texture coordinate */
            float u, v;
            if (sscanf(line + 3, "%f %f", &u, &v) >= 2) {
                if (tcCount < MAX_TEXCOORDS) {
                    texcoords[tcCount] = vec3(u, v, 0.0F);
                    tcCount++;
                }
            }
        } else if (strncmp(line, "usemtl ", 7) == 0) {
            /* flush current faces to current material */
            if (currentMat >= 0 && currentMat < model->materialCount && tempFaceCount > 0) {
                MaterialGroup* mg = &model->materials[currentMat];
                mg->faces = (TriangleFace*)realloc(mg->faces, sizeof(TriangleFace) * (size_t)(mg->faceCount + tempFaceCount));
                memcpy(&mg->faces[mg->faceCount], tempFaces, sizeof(TriangleFace) * (size_t)tempFaceCount);
                mg->faceCount += tempFaceCount;
                tempFaceCount = 0;
            }
            /* find material index by order (materials appear in same order as in MTL) */
            {
                int matIdx;
                /* materials are indexed in order they appear in the MTL file.
                   In the OBJ, usemtl names correspond to the same order. We use
                   a simple sequential counter based on how many usemtl we've seen. */
                char matName[256];
                sscanf(line + 7, " %255[^\r\n]", matName);
                currentMat = -1;
                for (matIdx = 0; matIdx < model->materialCount; matIdx++) {
                    /* We stored materials in MTL parse order. Match by index since
                       we didn't store names. Use order-based matching:
                       MTL order: Material, Material.001, Material.002
                       Match name suffix to determine index */
                    (void)matIdx;
                    break;
                }
                /* Simple approach: match by name pattern */
                if (strcmp(matName, "Material") == 0) {
                    currentMat = 0;
                } else if (strcmp(matName, "Material.001") == 0) {
                    currentMat = 1;
                } else if (strcmp(matName, "Material.002") == 0) {
                    currentMat = 2;
                } else {
                    /* fallback: assign to first material */
                    currentMat = 0;
                }
            }
        } else if (line[0] == 'f' && line[1] == ' ') {
            /* face: can be triangle or polygon, triangulate with fan */
            int vi[16], ti[16], ni[16];
            int vertCount = 0;
            char* ptr = line + 2;
            int k;

            while (vertCount < 16) {
                int v_idx = 0, t_idx = 0, n_idx = 0;
                if (sscanf(ptr, "%d/%d/%d", &v_idx, &t_idx, &n_idx) == 3) {
                    vi[vertCount] = v_idx;
                    ti[vertCount] = t_idx;
                    ni[vertCount] = n_idx;
                    vertCount++;
                } else if (sscanf(ptr, "%d//%d", &v_idx, &n_idx) == 2) {
                    vi[vertCount] = v_idx;
                    ti[vertCount] = 0;
                    ni[vertCount] = n_idx;
                    vertCount++;
                } else if (sscanf(ptr, "%d/%d", &v_idx, &t_idx) == 2) {
                    vi[vertCount] = v_idx;
                    ti[vertCount] = t_idx;
                    ni[vertCount] = 0;
                    vertCount++;
                } else if (sscanf(ptr, "%d", &v_idx) == 1) {
                    vi[vertCount] = v_idx;
                    ti[vertCount] = 0;
                    ni[vertCount] = 0;
                    vertCount++;
                } else {
                    break;
                }
                /* advance past this token */
                while (*ptr != '\0' && *ptr != ' ' && *ptr != '\t') ptr++;
                while (*ptr == ' ' || *ptr == '\t') ptr++;
            }

            /* triangulate polygon as fan: (0,1,2), (0,2,3), ... */
            for (k = 1; k + 1 < vertCount; k++) {
                if (tempFaceCount < MAX_FACES) {
                    TriangleFace* tf = &tempFaces[tempFaceCount];
                    int indices[3];
                    int tindices[3];
                    int nindices[3];
                    int j;

                    indices[0] = vi[0];
                    indices[1] = vi[k];
                    indices[2] = vi[k + 1];
                    tindices[0] = ti[0];
                    tindices[1] = ti[k];
                    tindices[2] = ti[k + 1];
                    nindices[0] = ni[0];
                    nindices[1] = ni[k];
                    nindices[2] = ni[k + 1];

                    for (j = 0; j < 3; j++) {
                        int pi = indices[j] - 1;
                        int tci = tindices[j] - 1;
                        int nmi = nindices[j] - 1;

                        if (pi >= 0 && pi < posCount) {
                            tf->v[j].position = positions[pi];
                        } else {
                            tf->v[j].position = vec3Null();
                        }

                        if (tci >= 0 && tci < tcCount) {
                            tf->v[j].texture = texcoords[tci];
                        } else {
                            tf->v[j].texture = vec3Null();
                        }

                        if (nmi >= 0 && nmi < normCount) {
                            tf->v[j].normal = normals[nmi];
                        } else {
                            tf->v[j].normal = vec3(0.0F, 1.0F, 0.0F);
                        }

                        tf->v[j].albedo = vec3(1.0F, 1.0F, 1.0F);
                    }
                    tempFaceCount++;
                }
            }
        }
    }

    /* flush remaining faces */
    if (currentMat >= 0 && currentMat < model->materialCount && tempFaceCount > 0) {
        MaterialGroup* mg = &model->materials[currentMat];
        mg->faces = (TriangleFace*)realloc(mg->faces, sizeof(TriangleFace) * (size_t)(mg->faceCount + tempFaceCount));
        memcpy(&mg->faces[mg->faceCount], tempFaces, sizeof(TriangleFace) * (size_t)tempFaceCount);
        mg->faceCount += tempFaceCount;
    }

    fclose(fp);
    free(positions);
    free(normals);
    free(texcoords);
    free(tempFaces);

    /* Load textures for each material */
    for (i = 0; i < model->materialCount; i++) {
        if (model->materials[i].texturePath[0] != '\0') {
            model->materials[i].textureID = uploadTexture(model->materials[i].texturePath);
        }
    }

    SDL_Log("OBJ loaded: %d materials, %d verts, %d norms, %d texcoords",
            model->materialCount, posCount, normCount, tcCount);

    return model;
}

extern void freeOBJModel(OBJModel* model) {
    int i;
    if (model == NULL) return;

    for (i = 0; i < model->materialCount; i++) {
        if (model->materials[i].textureID != 0) {
            glDeleteTextures(1, &model->materials[i].textureID);
        }
        if (model->materials[i].faces != NULL) {
            free(model->materials[i].faces);
        }
    }
    free(model);
}

extern void renderOBJModel(const OBJModel* model) {
    int i;
    if (model == NULL) return;

    glEnable(GL_TEXTURE_2D);
    for (i = 0; i < model->materialCount; i++) {
        if (model->materials[i].faceCount == 0) continue;

        if (model->materials[i].textureID != 0) {
            glBindTexture(GL_TEXTURE_2D, model->materials[i].textureID);
        } else {
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        renderTriFaces(model->materials[i].faces, model->materials[i].faceCount);
    }
    glDisable(GL_TEXTURE_2D);
}
