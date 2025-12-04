#pragma once

#include "Event.h"


#include <string>

namespace VTA {

	class Module
	{
	public:
		Layer(const std::string& name = "Module");
		virtual ~Module();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(Timestep ts) {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& event) {}

		inline const std::string& GetName() const { return m_DebugName; }
	protected:
		std::string m_DebugName;
	};

}
