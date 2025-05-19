#pragma once
#include <memory>
#include "serpent/core/IProcessReader.h"

namespace serpent::core {

/**
 * Creates a ProcessReader suitable for the current OS.
 * @return IProcessReader on success, nullptr on error.
 * @throw std::runtime_error if plugin loading fails.
 */
std::unique_ptr<IProcessReader> create_reader_for_current_os();

} // namespace serpent::core
