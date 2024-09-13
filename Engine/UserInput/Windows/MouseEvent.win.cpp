#include "../MouseEvent.h"

eae6320::UserInput::MouseEvent::MouseEvent()
	:
	type(EventType::Invalid),
	m_x(0),
	m_y(0)
{}

eae6320::UserInput::MouseEvent::MouseEvent(EventType type, int i_x, int i_y)
	:
	type(type),
	m_x(i_x),
	m_y(i_y)
{}

bool eae6320::UserInput::MouseEvent::IsValid() const
{
	return  type != EventType::Invalid;
}

eae6320::UserInput::MouseEvent::EventType eae6320::UserInput::MouseEvent::GetType() const
{
	return  type;
}

eae6320::UserInput::MousePoint eae6320::UserInput::MouseEvent::GetPos() const
{
	return{ m_x, m_y };
}

int eae6320::UserInput::MouseEvent::GetPosX() const
{
	return m_x;
}

int eae6320::UserInput::MouseEvent::GetPosY() const
{
	return m_y;
}