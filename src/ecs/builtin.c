#include "builtin.h"
#include "ecs.h"

uint32_t VE_TestComponentID = 0;

void VE_TestComponent_System(VE_EntityT *entity, void *data) {
	VE_TestComponent* component = (VE_TestComponent *)data;
	printf("Updating component with value %i\n", component->counter);
	component->counter++;
}

VE_TestComponent VE_NewTestComponent(int counter) {
	VE_TestComponent testComponent = { VE_TestComponentID, counter };
	return testComponent;
}

void VE_SetupBuiltinComponents() {
	VE_TestComponentID = VE_RegisterComponent(&VE_TestComponent_System, sizeof(VE_TestComponent));
}
