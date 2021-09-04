#include <vector>

class Skull;
class SceneObject;
typedef struct ICPSettings ICPSettings;

#ifndef ALIGN_H
#define ALGIN_H

int align(Skull& target, Skull& source, const ICPSettings& settings, std::vector<SceneObject*>& output_objects);

#endif