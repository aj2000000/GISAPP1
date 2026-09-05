# Documentation Standards Rule

## Objective
Enforce comprehensive, high-quality Doxygen and JSDoc documentation across all generated and modified code.

## Requirements

### 1. Class & File Header Documentation
Every class declaration and file must include Doxygen comments:
```cpp
/**
 * @file ClassName.h
 * @brief Brief summary of the component.
 *
 * Detailed explanation of the class responsibility, architectural placement,
 * and design patterns used.
 */

/**
 * @class ClassName
 * @brief Concise description of the class role.
 *
 * Detailed explanation of the component, its lifecycle, threading guarantees,
 * and interaction with collaborators.
 */
```

### 2. Method & Function Documentation
Every constructor, method, signal, slot, and helper function must include:
- `@brief`: Clear statement of purpose.
- `@param [in/out] <name>`: Explanation of purpose, expected range, and constraints.
- `@return`: Expected return values and failure states.
- `@note`: Special considerations, side-effects, or performance implications.

### 3. Qt-Specific Annotations
- Document Qt signals (`@signals`) indicating when and why they are fired.
- Document Qt slots (`@slots`) and whether they are invoked synchronously or asynchronously.
