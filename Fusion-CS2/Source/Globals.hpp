/*

Globals.hpp (Some variables used globally to configure the cheat)
Authors: 0Zayn (Zayn)

*/

#pragma once

namespace Globals {
	namespace Interface {
		inline bool Open = true;
	}

	namespace ESP {
		inline bool Enabled = false;

		inline bool Box = false;

		inline bool Health = false;
		inline bool HealthText = false;
	}

	namespace Aimbot {
		inline bool Enabled = false;

		inline float Smoothing = 0.5f;
		inline bool StickyAim = false;
	}
}