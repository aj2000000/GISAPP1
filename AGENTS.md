# Agent Rules & Guidelines

## Documentation Standards

When generating class code snippets, definitions, or implementations:
- Include detailed **Doxygen** (for C++/C) or **JSDoc** (for JavaScript/TypeScript) style documentation.
- **Class-Level Documentation**:
  - `@file`: File name and brief overview.
  - `@class`: Class name and architectural role/responsibility.
  - Detailed explanation of how the class interacts with other components, subsystems, or design patterns.
- **Function & Method Documentation**:
  - `@brief`: Concise summary of what the function accomplishes.
  - `@param`: Description of each parameter, including valid ranges, ownership, or preconditions where applicable.
  - `@return`: Description of return values, return types, or status indicators.
  - `@note` / `@warning`: Any side effects, threading assumptions, or critical behaviors.
  - `@signals` / `@slots`: For Qt classes, document signals emitted and slot handlers.
- **Member Variables**:
  - Provide inline or block comments describing the purpose and constraints of member variables.
