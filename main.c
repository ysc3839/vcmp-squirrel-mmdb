#include "main.h"

static void vcmp_on_squirrel_script_load(void)
{
	// See if we have any imports from Squirrel
	size_t size;
	int32_t sq_id = vcmp_functions->FindPlugin("SQHost2");
	if (sq_id != -1) // vcmpEntityNone
	{
		const void** sq_exports = vcmp_functions->GetPluginExports(sq_id, &size);

		// We do!
		if (sq_exports != NULL && size > 0)
		{
			// Cast to a SquirrelImports structure
			SquirrelImports** sq_deref_funcs = (SquirrelImports**)sq_exports;

			// Now let's change that to a SquirrelImports pointer
			SquirrelImports* sq_funcs = *sq_deref_funcs;

			// Now we get the virtual machine
			if (sq_funcs)
			{
				// Get a pointer to the VM and API
				sq = *(sq_funcs->GetSquirrelAPI());
				HSQUIRRELVM v = *(sq_funcs->GetSquirrelVM());

				register_sq_funcs(v);
			}
		}
		else
			output_console_message("Failed to attach to SQHost2.");
	}
}

static uint8_t vcmp_on_server_initialise(void)
{
#define _STR(s) #s
#define STR(s) _STR(s)
	output_console_message("Loaded SqMMDB plugin for SqVCMP by ysc3839. (plugin api: " STR(PLUGIN_API_MAJOR) "." STR(PLUGIN_API_MINOR) ")");
	return true;
}

static uint8_t vcmp_on_plugin_command(uint32_t command_identifier, const char *message)
{
	switch (command_identifier)
	{
	case 0x7D6E22D8:
		vcmp_on_squirrel_script_load();
		break;
	}
	return true;
}

EXPORT uint32_t VcmpPluginInit(PluginFuncs* functions, PluginCallbacks* callbacks, PluginInfo* info)
{
	info->pluginVersion = 0x100; // 1.0.0
	strncpy(info->name, "vcmp-squirrel-mmdb", sizeof(info->name));

	info->apiMajorVersion = PLUGIN_API_MAJOR;
	info->apiMinorVersion = PLUGIN_API_MINOR;

	vcmp_functions = functions;

	callbacks->OnServerInitialise = vcmp_on_server_initialise;
	callbacks->OnPluginCommand = vcmp_on_plugin_command;

	return 1;
}
