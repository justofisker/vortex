#ifndef RENDER_INIT_H
#define RENDER_INIT_H

void VE_Render_CreateInstance();
void VE_Render_PickPhysicalDeviceAndQueues();
void VE_Render_CreateDevice();
void VE_Render_CreateSurface();
void VE_Render_CreateSwapchain();
void VE_Render_CreateCommandPool();
void VE_Render_CreateSyncObjects();
void VE_Render_CreateRenderpass();
void VE_Render_CreateFramebuffers();

#ifndef NDEBUG
int VE_Render_CheckValidationSupport();
void VE_Render_RegisterDebugReportCallback();
#endif // NDEBUG

#endif // RENDER_INIT_H
