

namespace eae6320
{
	namespace UserInput
	{

		struct MousePoint
		{
			int x;
			int y;
		};

		class MouseEvent
		{
		public:
			enum EventType
			{
				LPress,
				LRelease,
				RPress,
				RRelease,
				MPress,
				MRelease,
				WheelUp,
				WheelDown,
				Move,
				Invalid
			};
		private:
			EventType type;
			int m_x;
			int m_y;
		public:
			MouseEvent();
			MouseEvent(const EventType type, const int i_x, const int i_y);
			bool IsValid() const;
			EventType GetType() const;
			MousePoint GetPos() const;
			int GetPosX() const;
			int GetPosY() const;
		};
	}
}