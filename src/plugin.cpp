// NoIntroAutoStart — skips the "PRESS (SPACE) TO START" gate on launch.
//
// The gate only triggers the platform login. On the first Map_MainMenu, once the
// startup widget exists, invoke CommonUserSubsystem.TryToLoginForOnlinePlay(0) and
// the widget's SwitchToFullMenu() by name. SwitchToFullMenu hides press-start
// without waiting for the async login. One-shot; all game calls resolved by name.

#include <cstdint>
#include <cstring>

#include "plugin.h"
#include "plugin_helpers.h"
#include "plugin_config.h"

// Param layouts must match the native UFunction Params exactly (ProcessEvent does
// no marshaling). From SDK CommonUser_parameters.hpp: 0x8 bytes, return at 0x4.
struct Params_TryToLoginForOnlinePlay
{
	int32_t LocalPlayerIndex; // 0x0
	bool    ReturnValue;      // 0x4
	uint8_t Pad_5[3];         // 0x5
};

struct Params_GetLocalPlayerInitializationState
{
	int32_t LocalPlayerIndex; // 0x0
	uint8_t ReturnValue;      // 0x4  (ECommonUserInitializationState : uint8)
	uint8_t Pad_5[3];         // 0x5
};

// ECommonUserInitializationState values (SDK CommonUser_structs.hpp).
enum : uint8_t
{
	CU_Unknown           = 0,
	CU_DoingInitialLogin = 1,
	CU_DoingNetworkLogin = 2,
	CU_FailedToLogin     = 3,
	CU_LoggedInOnline    = 4,
	CU_LoggedInLocalOnly = 5,
	CU_Invalid           = 6,
};

static const char* kSubsystemClass = "CommonUserSubsystem";
static const char* kWidgetClass    = "WBP_StartupScreen_C";
static const char* kFnLogin        = "TryToLoginForOnlinePlay";
static const char* kFnInitState    = "GetLocalPlayerInitializationState";
static const char* kFnSwitchMenu   = "SwitchToFullMenu";

// Login completes in ~0.2 s; the 10 s timeout is a large margin.
static constexpr float kWidgetWaitTimeout = 20.0f;
static constexpr float kLoginTimeout      = 10.0f;
static constexpr float kWalkInterval      = 0.05f;
static constexpr float kPollInterval      = 0.10f;

static IPluginSelf* g_self = nullptr;
IPluginSelf* GetSelf() { return g_self; }

#ifndef MODLOADER_BUILD_TAG
#define MODLOADER_BUILD_TAG "dev"
#endif

static PluginInfo s_pluginInfo = {
	"AgenticalMods-NoIntroAutoStart",
	MODLOADER_BUILD_TAG,
	"AgenticalMods",
	"Advances past the PRESS (SPACE) TO START screen to the main menu automatically.",
	PLUGIN_INTERFACE_VERSION,
	PLUGIN_TARGET_CLIENT
};

// One-shot state machine. All state below is game-thread only (OnAnyWorldBeginPlay
// and OnTick both run under the engine tick), so it needs no locking.
enum class State { Idle, WaitingForWidget, WaitingForLogin, Done, StoodDown };

static State g_state       = State::Idle;
static float g_sinceArm    = 0.0f;    // seconds since arming
static float g_walkTimer   = 0.0f;    // countdown between GObjects walks
static float g_sinceLogin  = 0.0f;    // seconds since the login request
static float g_pollTimer   = 0.0f;    // countdown between init-state polls
static void* g_subsystem   = nullptr; // CommonUserSubsystem, session-stable
static int   g_lastState   = -1;      // last-logged init state

static const char* InitStateName(int s)
{
	switch (s)
	{
	case CU_Unknown:           return "Unknown";
	case CU_DoingInitialLogin: return "DoingInitialLogin";
	case CU_DoingNetworkLogin: return "DoingNetworkLogin";
	case CU_FailedToLogin:     return "FailedToLogin";
	case CU_LoggedInOnline:    return "LoggedInOnline";
	case CU_LoggedInLocalOnly: return "LoggedInLocalOnly";
	case CU_Invalid:           return "Invalid";
	default:                   return "?";
	}
}

// Abort before any UI change: the boot proceeds as vanilla.
static void StandDown(const char* why)
{
	LOG_WARN("Standing down (%s). Boot proceeds as vanilla — press SPACE to start.", why);
	g_state = State::StoodDown;
	g_subsystem = nullptr;
}

// First live (non-CDO) instance of a class short-name, or null. Walks GObjects —
// expensive; callers rate-limit.
static void* FindFirstInstanceByClass(const char* className)
{
	IPluginObjectWalker* ow = (g_self && g_self->hooks) ? g_self->hooks->ObjectWalker : nullptr;
	if (!ow || !ow->IsReady())
		return nullptr;

	PluginObjectInfo buf[8];
	int n = ow->FindObjectsByClassNameInto(className, PluginObjectLookup_InstanceOnly, buf, 8);
	if (n <= 0)
		return nullptr;
	return buf[0].object; // count may exceed 8; buf[0] is valid
}

static void OnEngineInit()
{
	LOG_INFO("EngineInit reached (GObjects ready).");
}

static void OnAnyWorldBeginPlay(SDK::UWorld* /*world*/, const char* worldName)
{
	LOG_INFO("WorldBeginPlay: %s", worldName ? worldName : "(null)");
	// Arm once: press-start only appears on first launch, not on return to menu.
	if (g_state == State::Idle && worldName && std::strstr(worldName, "Map_MainMenu"))
	{
		g_state     = State::WaitingForWidget;
		g_sinceArm  = 0.0f;
		g_walkTimer = 0.0f;
		LOG_INFO("Map_MainMenu reached — arming auto-start (waiting for the startup widget).");
	}
}

static void OnTick(float dt)
{
	switch (g_state)
	{
	case State::WaitingForWidget:
	{
		g_sinceArm  += dt;
		g_walkTimer -= dt;
		if (g_sinceArm > kWidgetWaitTimeout)
		{
			StandDown("startup widget never appeared");
			return;
		}
		if (g_walkTimer > 0.0f)
			return;
		g_walkTimer = kWalkInterval;

		// Once the widget exists its Construct has run, so its login-complete
		// handler is bound and it will advance on login.
		void* widget = FindFirstInstanceByClass(kWidgetClass);
		if (!widget)
			return;

		void* subsystem = FindFirstInstanceByClass(kSubsystemClass);
		if (!subsystem)
		{
			StandDown("CommonUserSubsystem instance not found");
			return;
		}

		IPluginObjectWalker* ow = g_self->hooks->ObjectWalker;
		Params_TryToLoginForOnlinePlay p{};
		p.LocalPlayerIndex = 0;
		p.ReturnValue      = false;
		bool ok = ow->InvokeUFunctionByName(subsystem, kSubsystemClass, kFnLogin, &p);
		if (!ok)
		{
			StandDown("TryToLoginForOnlinePlay could not be invoked (patched game?)");
			return;
		}

		// Hide press-start now rather than waiting for the async login.
		// SwitchToFullMenu only changes the panel, so it is safe before login.
		bool sw = ow->InvokeUFunctionByName(widget, kWidgetClass, kFnSwitchMenu, nullptr);

		g_subsystem   = subsystem;
		g_state       = State::WaitingForLogin;
		g_sinceLogin  = 0.0f;
		g_pollTimer   = 0.0f;
		g_lastState   = -1;
		LOG_INFO("Login requested: CommonUserSubsystem.TryToLoginForOnlinePlay(0) (immediate return=%d); "
		         "SwitchToFullMenu %s. Login completes in the background.",
		         (int)p.ReturnValue, sw ? "invoked (press-start hidden immediately)"
		                                : "FAILED (widget will advance on login-complete instead)");
		return;
	}

	case State::WaitingForLogin:
	{
		g_sinceLogin += dt;
		g_pollTimer  -= dt;
		if (g_pollTimer <= 0.0f)
		{
			g_pollTimer = kPollInterval;
			IPluginObjectWalker* ow = g_self->hooks->ObjectWalker;
			Params_GetLocalPlayerInitializationState q{};
			q.LocalPlayerIndex = 0;
			q.ReturnValue      = CU_Unknown;
			if (ow->InvokeUFunctionByName(g_subsystem, kSubsystemClass, kFnInitState, &q))
			{
				// Not 's': the LOG_* macros declare their own 's' and would shadow it.
				int st = (int)q.ReturnValue;
				if (st != g_lastState)
				{
					g_lastState = st;
					LOG_INFO("Login state: %s (%d)", InitStateName(st), st);
				}
				if (st == CU_LoggedInOnline || st == CU_LoggedInLocalOnly)
				{
					LOG_INFO("Login complete — the main menu should now be shown. Done.");
					g_state = State::Done;
					g_subsystem = nullptr;
					return;
				}
				if (st == CU_FailedToLogin || st == CU_Invalid)
				{
					// Menu already shown; a login failure here just means offline.
					LOG_WARN("Login reported failure (%s); the menu is shown but "
					         "online/save features may be unavailable.", InitStateName(st));
					g_state = State::Done;
					g_subsystem = nullptr;
					return;
				}
			}
		}
		if (g_sinceLogin > kLoginTimeout)
		{
			// Give up polling after the timeout; no retry.
			LOG_WARN("Login did not report completion within %.0fs; stopping poll "
			         "(the login request was still sent).", kLoginTimeout);
			g_state = State::Done;
			g_subsystem = nullptr;
		}
		return;
	}

	default:
		return;
	}
}

extern "C" {

	__declspec(dllexport) PluginInfo* GetPluginInfo()
	{
		return &s_pluginInfo;
	}

	__declspec(dllexport) bool PluginInit(IPluginSelf* self)
	{
		g_self = self;

		LOG_INFO("NoIntroAutoStart initializing (build %s, interface v%d)...",
			s_pluginInfo.version, s_pluginInfo.interfaceVersion);

		NoIntroConfig::Config::Initialize(self);
		if (!NoIntroConfig::Config::IsEnabled())
		{
			LOG_WARN("Disabled in config; standing down (vanilla boot).");
			return true;
		}

		if (!self || !self->hooks)
		{
			LOG_ERROR("No hooks interface; cannot run.");
			return true;
		}
		if (!self->hooks->ObjectWalker) // by-name invoke needs v47+
		{
			LOG_ERROR("ObjectWalker unavailable (loader too old); standing down.");
			return true;
		}

		if (self->hooks->Engine)
		{
			self->hooks->Engine->RegisterOnInit(OnEngineInit);
			self->hooks->Engine->RegisterOnTick(OnTick);
		}
		if (self->hooks->World)
			self->hooks->World->RegisterOnAnyWorldBeginPlay(OnAnyWorldBeginPlay);

		LOG_INFO("NoIntroAutoStart initialized; watching for Map_MainMenu.");
		return true;
	}

	__declspec(dllexport) void PluginShutdown()
	{
		if (g_self && g_self->hooks)
		{
			if (g_self->hooks->Engine)
			{
				g_self->hooks->Engine->UnregisterOnInit(OnEngineInit);
				g_self->hooks->Engine->UnregisterOnTick(OnTick);
			}
			if (g_self->hooks->World)
				g_self->hooks->World->UnregisterOnAnyWorldBeginPlay(OnAnyWorldBeginPlay);
		}
		LOG_INFO("NoIntroAutoStart shutting down.");
		g_state = State::Idle;
		g_subsystem = nullptr;
		g_self = nullptr;
	}

} // extern "C"
