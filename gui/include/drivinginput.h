// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL
#ifndef CHIAKI_DRIVINGINPUT_H
#define CHIAKI_DRIVINGINPUT_H

#include <chiaki/controller.h>
#include <algorithm>
#include <set>

// Track physical keys so mode changes and opposing directions cannot leave a
// stale stick or trigger held. Apply before merging mouse/controller input.
class DrivingInput
{
	bool enabled = false;
	std::set<int> forward_keys;
	std::set<int> backward_keys;

public:
	bool Toggle() { enabled = !enabled; return enabled; }
	void Clear() { forward_keys.clear(); backward_keys.clear(); }
	void HandleKey(int key, bool forward, bool pressed)
	{
		forward_keys.erase(key);
		backward_keys.erase(key);
		if(pressed)
			(forward ? forward_keys : backward_keys).insert(key);
	}
	void Apply(ChiakiControllerState &state) const
	{
		const bool forward = !forward_keys.empty();
		const bool backward = !backward_keys.empty();
		state.left_y = enabled ? 0 : (int(backward) - int(forward)) * 0x7fff;
		if(enabled)
		{
			state.r2_state = std::max(state.r2_state, uint8_t(forward ? 0xff : 0));
			state.l2_state = std::max(state.l2_state, uint8_t(backward ? 0xff : 0));
		}
	}
};

#endif
