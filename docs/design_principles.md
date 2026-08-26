# Mercury Design Principles

## Correctness before optimization.
Mercury prioritizes maintaining a valid market state over raw performance. Every operation must preserve order book invariants before optimization is considered. 
Performance improvements are only accepted after correctness is established through unit tests,replay validation, and sanitizer checks.

## Every abstraction must model a real market concept.
Types and components should correspond to concepts that exist in an exchange or professional limit order book. 
This principle keeps the design intutive and prevents unnecessary abstractions.

## Measure before optimizing.
Performance work should be driven by data rather than assumptions.
Example : Historical replay throughput is measured by LOBSTER datasets and Operation latency distributions are measured using dedicated benchmarks.

## Identity is immutable.
An order's identity must never change during its lifetime. This includes : 
- OrderID must remain constant from submission until removal.
- State transitions modify order status and quantity but never identity.
Stable identities simplify reasoning and prevent synchronization errors.

## Ownership is explicit.
Every object has a clear defined owner and lifetime.
Examples :
- OrderBook owns active orders through well-defined storage.
- Price levels do not assume ownership of orders.
- Location tracking is maintained explicity through OrderLocation.
This reduces ambiguity and helps avoid memory-management bugs.

## Prefer composition over inheritance.
Behaviour is built by combining focused components rather than creating deep inheritance hierarchies. 
For example : OrderBook is composed of two BookSide instances and BookSide manages multiple PriceLevel objects.
Composition improves maintainability and keeps dependencies localized.

## APIs should be difficult to misuse.
Invalid operations should be prevented whenever possible. 
Examples :
- Strongly typed wrappers are used for identifiers, price, quantities and timestamps.
- Assertions enforce preconditions and invariants.
- Invalid state transitions are rejected.
"The easiest way to use an API should also be the correct way"

## Document design decisions, not just code.
Implementation details explain `how` the system works, documentation explains `why` it was built this way.
Well-documented decisions remain valuable long after indiviual code changes.

## Determinism matters
Given the same sequence of events, Mercury should always produce the same resulting market state.
This includes :
- Price-time priority is being strictly enforced.
- Replay processing is deterministic.
- Unit tests relying on reproducible outcomes.
Determinism simplifies testing, debugging, validation and benchmarking.

## Simple design win until proven otherwise 
New complexity must justify itself through measurable benefits.
Examples : 
- Standard library containers are used until profiling identifies a bottleneck.
- Specialised data structures (like intrusive linked-lists) are introduced only after comparison against simpler alternatives.
- Architectural complexity should be earned through evidence.
This priniciple keeps the system understandable while leaving room for future optimisation.