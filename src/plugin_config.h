#pragma once

#include "plugin_interface.h"

// One knob: a master Enabled toggle, so the plugin can be turned off without
// removing the DLL.
namespace NoIntroConfig
{
	static const ConfigEntry CONFIG_ENTRIES[] = {
		{
			"General",
			"Enabled",
			ConfigValueType::Boolean,
			"true",
			"Enable NoIntro auto-start (advance past PRESS (SPACE) TO START to the main menu)."
		}
	};

	static const ConfigSchema SCHEMA = {
		CONFIG_ENTRIES,
		sizeof(CONFIG_ENTRIES) / sizeof(ConfigEntry)
	};

	class Config
	{
	public:
		static void Initialize(IPluginSelf* self)
		{
			s_self = self;
			if (s_self)
			{
				s_self->config->InitializeFromSchema(s_self, &SCHEMA);
			}
		}

		static bool IsEnabled()
		{
			return s_self ? s_self->config->ReadBool(s_self, "General", "Enabled", true) : true;
		}

	private:
		static IPluginSelf* s_self;
	};
}
