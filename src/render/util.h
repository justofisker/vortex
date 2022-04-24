#ifndef RENDER_UTIL_H
#define RENDER_UTIL_H

void VE_Render_CreateInstance();
void VE_Render_PickPhysicalDeviceAndQueues();
void VE_Render_CreateDevice();
void VE_Render_CreateSurface();

#ifndef NDEBUG
int VE_Render_CheckValidationSupport();
void VE_Render_RegisterDebugReportCallback();
#endif // NDEBUG

#endif // RENDER_UTIL_H