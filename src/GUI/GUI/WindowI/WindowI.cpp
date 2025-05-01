#include "WindowI.hpp"

namespace GUI
{
WindowI::WindowI(std::string_view name) : m_name {name} {}

std::string_view WindowI::name() const { return m_name; }

} // namespace GUI
