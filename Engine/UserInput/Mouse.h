#pragma once
#include "MouseEvent.h"
#include <queue>


namespace eae6320
{
	namespace UserInput
	{

		class Mouse
		{
		public:
			void OnLeftPressed(int i_x, int i_y);
			void OnLeftReleased(int i_x, int i_y);
			void OnRightPressed(int i_x, int i_y);
			void OnRightReleased(int i_x, int i_y);
			void OnMiddlePressed(int i_x, int i_y);
			void OnMiddleReleased(int i_x, int i_y);
			void OnWheelUp(int i_x, int i_y);
			void OnWheelDown(int i_x, int i_y);
			void OnMouseMove(int i_x, int i_y);

			bool IsLeftDown();
			bool IsMiddleDown();
			bool IsRightDown();

			int GetPosX();
			int GetPosY();
			int GetPrePosX();
			int GetPrePosY();
			MousePoint GetPos();

			bool EventBufferIsEmpty();
			MouseEvent ReadEvent();

		private:
			std::queue<MouseEvent> eventBuffer;
			bool leftIsDown = false;
			bool rightIsDown = false;
			bool mbuttonDown = false;
			int m_x = 0;
			int m_y = 0;
			int m_pre_x = 0;
			int m_pre_y = 0;
		};
	}
}