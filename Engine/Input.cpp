#include "pch.h"
#include "Input.h"
#include "Engine.h"

void Input::Init(HWND hwnd)
{
	m_hwnd = hwnd;
	m_states.resize(KEY_TYPE_COUNT, KEY_STATE::NONE);
}

void Input::Update()
{
	HWND hwnd = ::GetActiveWindow();
	if (m_hwnd != hwnd) {
		for (const KEY_TYPE key : ALL_KEYS)
			m_states[static_cast<int32>(key)] = KEY_STATE::NONE;
		return;
	}

	for (const KEY_TYPE key : ALL_KEYS) {
		// Ű�� ���� ������ true
		if (::GetAsyncKeyState(static_cast<int32>(key)) & 0x8000) {
			KEY_STATE& state = m_states[static_cast<int32>(key)];

			// ���� �����ӿ� Ű�� ���� ���¶�� PRESS
			if (state == KEY_STATE::PRESS || state == KEY_STATE::DOWN)
				state = KEY_STATE::PRESS;
			else
				state = KEY_STATE::DOWN;
		}
		else {
			KEY_STATE& state = m_states[static_cast<int32>(key)];

			// ���� �����ӿ� Ű�� ���� ���¶�� UP
			if (state == KEY_STATE::PRESS || state == KEY_STATE::DOWN)
				state = KEY_STATE::UP;
			else
				state = KEY_STATE::NONE;
		}
	}

	::GetCursorPos(&m_mousePos);
	::ScreenToClient(GEngine->GetWindow().hwnd, &m_mousePos);
}