// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL
#include "drivinginput.h"
#include <cstdlib>
#include <iostream>

static void check(bool condition)
{
	if(!condition)
		std::abort();
}

static ChiakiControllerState read(const DrivingInput &input)
{
	ChiakiControllerState state;
	chiaki_controller_state_set_idle(&state);
	input.Apply(state);
	return state;
}

int main()
{
	DrivingInput input;
	input.HandleKey('W', true, true);
	check(read(input).left_y == -32767 && read(input).r2_state == 0);
	check(input.Toggle());
	check(read(input).left_y == 0 && read(input).r2_state == 255);
	input.HandleKey('W', true, false);
	check(read(input).r2_state == 0);
	input.HandleKey('S', false, true);
	check(read(input).l2_state == 255);
	input.HandleKey('W', true, true);
	check(read(input).r2_state == 255 && read(input).l2_state == 255);
	check(!input.Toggle());
	check(read(input).left_y == 0 && read(input).r2_state == 0 && read(input).l2_state == 0);
	input.HandleKey('W', true, false);
	check(read(input).left_y == 32767);
	input.HandleKey('S', false, false);
	check(read(input).left_y == 0);

	// Duplicate presses, multiple bound keys and releasing just one key.
	input.Toggle();
	input.HandleKey('W', true, true);
	input.HandleKey('W', true, true);
	input.HandleKey('I', true, true);
	input.HandleKey('W', true, false);
	check(read(input).r2_state == 255);
	input.HandleKey('I', true, false);
	check(read(input).r2_state == 0);

	// Existing trigger bindings, steering, camera and buttons survive mapping.
	auto state = read(input);
	state.r2_state = 128;
	state.l2_state = 64;
	state.left_x = -20000;
	state.right_y = 12000;
	state.buttons = CHIAKI_CONTROLLER_BUTTON_R1;
	input.Apply(state);
	check(state.r2_state == 128 && state.l2_state == 64);
	check(state.left_x == -20000 && state.right_y == 12000);
	check(state.buttons == CHIAKI_CONTROLLER_BUTTON_R1);
	input.HandleKey('W', true, true);
	input.Apply(state);
	check(state.r2_state == 255);

	// Clearing on focus loss releases keyboard input; a held mouse trigger
	// remains independent when the actual controller states are merged.
	input.Clear();
	state = read(input);
	check(state.left_y == 0 && state.r2_state == 0 && state.l2_state == 0);
	ChiakiControllerState mouse;
	chiaki_controller_state_set_idle(&mouse);
	mouse.r2_state = 255;
	chiaki_controller_state_or(&state, &state, &mouse);
	check(state.r2_state == 255);

	// Exercise event sequences across every pair of held-direction states.
	for(int mask = 0; mask < 4; ++mask)
	{
		DrivingInput sample;
		sample.HandleKey('W', true, mask & 1);
		sample.HandleKey('S', false, mask & 2);
		for(int repeat = 0; repeat < 100; ++repeat)
		{
			sample.Toggle();
			auto driving = read(sample);
			check(driving.left_y == 0);
			check(driving.r2_state == ((mask & 1) ? 255 : 0));
			check(driving.l2_state == ((mask & 2) ? 255 : 0));
			sample.Toggle();
			check(read(sample).left_y == (int(bool(mask & 2)) - int(bool(mask & 1))) * 32767);
		}
	}
	std::cout << "Driving input regression tests passed\n";
}
