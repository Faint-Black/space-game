#ifndef OBJLOADER_H
#define OBJLOADER_H

#include "render.h"
#include "utils.h"
#include <GL/gl.h>

#define OBJ_MAX_MATERIALS 8

typedef struct MaterialGroup {
    char texturePath[256];
    GLuint textureID;
    TriangleFace* faces;
    int faceCount;
} MaterialGroup;

typedef struct OBJModel {
    MaterialGroup materials[OBJ_MAX_MATERIALS];
    int materialCount;
} OBJModel;

/**
 * @brief Loads a Wavefront OBJ model (with MTL materials and PNG textures).
 *
 * @param objPath Path to the .obj file.
 * @return Pointer to the loaded model, or NULL on failure.
 */
extern OBJModel* loadOBJModel(const char* objPath);

/**
 * @brief Frees all memory and OpenGL resources associated with a model.
 */
extern void freeOBJModel(OBJModel* model);

/**
 * @brief Renders the model using OpenGL fixed-function pipeline.
 */
extern void renderOBJModel(const OBJModel* model);

#endif /* OBJLOADER_H */
