#ifndef MODULE_H
#define MODULE_H

#include "utils.h"

typedef struct Module {
    Vec3 position;
    int  active;
} Module;

/**
 * @brief Initializes the rescue modules at random positions. Called once at
 *        startup and again on every restart.
 */
extern void initModules(void);

/**
 * @brief Fetches the constant reference to the module array.
 */
extern const Module* getModules(void);

/**
 * @brief Fetches the length of the module array.
 */
extern int getModuleCount(void);

/**
 * @brief Marks the module at the given index as collected (inactive).
 */
extern void collectModule(int module_id);

#endif /* MODULE_H */