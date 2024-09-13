#include "../Mouse.h"

void eae6320::UserInput::Mouse::OnLeftPressed(int i_x, int i_y)
{
	 leftIsDown = true;
	MouseEvent me(MouseEvent::EventType::LPress, i_x, i_y);
	 eventBuffer.push(me);
}

void eae6320::UserInput::Mouse::OnLeftReleased(int i_x, int i_y)
{
	 leftIsDown = false;
	 eventBuffer.push(MouseEvent(MouseEvent::EventType::LRelease, i_x, i_y));
}

void eae6320::UserInput::Mouse::OnRightPressed(int i_x, int i_y)
{
	 rightIsDown = true;
	 eventBuffer.push(MouseEvent(MouseEvent::EventType::RPress, i_x, i_y));
}

void eae6320::UserInput::Mouse::OnRightReleased(int i_x, int i_y)
{
	 rightIsDown = false;
	 eventBuffer.push(MouseEvent(MouseEvent::EventType::RRelease, i_x, i_y));
}

void eae6320::UserInput::Mouse::OnMiddlePressed(int i_x, int i_y)
{
	 mbuttonDown = true;
	 eventBuffer.push(MouseEvent(MouseEvent::EventType::MPress, i_x, i_y));
}

void eae6320::UserInput::Mouse::OnMiddleReleased(int i_x, int i_y)
{
	 mbuttonDown = false;
	 eventBuffer.push(MouseEvent(MouseEvent::EventType::MRelease, i_x, i_y));
}

void eae6320::UserInput::Mouse::OnWheelUp(int i_x, int i_y)
{
	 eventBuffer.push(MouseEvent(MouseEvent::EventType::WheelUp, i_x, i_y));
}

void eae6320::UserInput::Mouse::OnWheelDown(int i_x, int i_y)
{
	 eventBuffer.push(MouseEvent(MouseEvent::EventType::WheelDown, i_x, i_y));
}

void eae6320::UserInput::Mouse::OnMouseMove(int i_x, int i_y)
{
	m_pre_x = m_x;
	m_pre_y = m_y;

	 m_x = i_x;
	 m_y = i_y;
	 eventBuffer.push(MouseEvent(MouseEvent::EventType::Move, i_x, i_y));
}

bool eae6320::UserInput::Mouse::IsLeftDown()
{
	return  leftIsDown;
}

bool eae6320::UserInput::Mouse::IsMiddleDown()
{
	return  mbuttonDown;
}

bool eae6320::UserInput::Mouse::IsRightDown()
{
	return  rightIsDown;
}

int eae6320::UserInput::Mouse::GetPosX()
{
	return m_x;
}

int eae6320::UserInput::Mouse::GetPosY()
{
	return m_y;
}

int eae6320::UserInput::Mouse::GetPrePosX()
{
	return m_pre_x;
}

int eae6320::UserInput::Mouse::GetPrePosY()
{
	return m_pre_y;
}

eae6320::UserInput::MousePoint eae6320::UserInput::Mouse::GetPos()
{
	return{ m_x, m_y };
}

bool eae6320::UserInput::Mouse::EventBufferIsEmpty()
{
	return  eventBuffer.empty();
}

eae6320::UserInput::MouseEvent eae6320::UserInput::Mouse::ReadEvent()
{
	if ( eventBuffer.empty())
	{
		return MouseEvent();
	}
	else
	{
		MouseEvent e =  eventBuffer.front(); //Get first event from buffer
		eventBuffer.pop(); //Remove first event from buffer
		return e;
	}
}