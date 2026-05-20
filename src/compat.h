#pragma once

#if defined(__CUDACC__) and defined(GPU_RENDER)

#include <cuda/std/optional>
#include <cuda/std/variant>

namespace compat {

using ::cuda::std::get;
using ::cuda::std::get_if;
using ::cuda::std::holds_alternative;
using ::cuda::std::variant;
using ::cuda::std::visit;

using ::cuda::std::decay_t;
using ::cuda::std::is_same_v;

using ::cuda::std::nullopt;
using ::cuda::std::optional;

}  // namespace compat

#else

#include <optional>
#include <variant>

namespace compat {

using ::std::get;
using ::std::get_if;
using ::std::holds_alternative;
using ::std::variant;
using ::std::visit;

using ::std::decay_t;
using ::std::is_same_v;

using ::std::nullopt;
using ::std::optional;

}  // namespace compat

#endif
