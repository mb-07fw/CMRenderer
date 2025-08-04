#pragma once

namespace CMEngine::Platform
{
	struct WindowFuncTable
	{
		using BoolFunc = bool (*)();
		using VoidFunc = void (*)();

		BoolFunc IsRunning = nullptr;
		BoolFunc ShouldClose = nullptr;
		VoidFunc Update = nullptr;

		inline WindowFuncTable(
			BoolFunc isRunningFunc,
			BoolFunc shouldCloseFunc
		) noexcept
			: IsRunning(isRunningFunc),
			  ShouldClose(shouldCloseFunc)
		{
		}
	};

	class IWindow
	{
	public:
		IWindow(const WindowFuncTable& funcTable) noexcept;
		virtual ~IWindow() = default;

		IWindow(const IWindow& other) = delete;
		IWindow& operator=(const IWindow& other) = delete;
	public:
		inline void Update() noexcept { return m_FuncTable.Update(); }

		inline [[nodiscard]] bool IsRunning() const noexcept { return m_FuncTable.IsRunning(); }
		inline [[nodiscard]] bool ShouldClose() const noexcept { return m_FuncTable.ShouldClose(); }
	private:
		WindowFuncTable m_FuncTable;
	};
}