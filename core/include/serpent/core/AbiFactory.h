#pragma once
#include <memory>
#include "serpent/core/IPythonABI.h"
#include "serpent/core/IProcessReader.h"

namespace serpent {

/**
 * Loads the ABI plugin for the specified Python version.
 * @param major The major version of Python (e.g., 3)
 * @param minor The minor version of Python (e.g., 10)
 * @param reader The memory reader for the target process
 * @return The IPythonABI interface on success, or nullptr on failure
 */
std::unique_ptr<IPythonABI> create_abi_for_version(int major, int minor, IProcessReader* reader);

} // namespace serpent
