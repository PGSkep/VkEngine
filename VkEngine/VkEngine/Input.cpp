#include "Input.h"

Input::INPUT_STATE Input::inputPressData[256];
uint32_t Input::mousePosition[2];
int32_t Input::mouseDelta[2];
std::vector<Input::KEYS>* Input::activeKeys = nullptr;