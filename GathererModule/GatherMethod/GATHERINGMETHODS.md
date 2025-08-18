## NormalGathering

Short description:
- Slot-based, timer-driven gathering method. Requests a slot at the resource, moves to the slot if out of range, gathers over time, harvests one logical stack per cycle, and re-enters the loop for the next decision (continue vs deposit) via the module’s single entrypoint.

Attributes:
- Storage policy: method-local stacks
  - `StacksStorageAmount`: max stacks to carry (default 1)
  - `CurrentGatheredStacks`: current carried stacks (resets after deposit)
- Target readiness:
  - Caches `GatherableModule` per `CurrentGatheringTarget`
  - Uses `GatherableModule->GatheringTime` per cycle
- Movement & range:
  - Requests slot via `SlotModule`
  - Acceptance radius: 10 units (starts gathering if already in range)
  - Movement is executed by `UGathererModule::MoveToLocation`
  - On movement success, `UGathererModule` re-enters `ExecuteGather()`
- Gathering loop:
  - Tick interval: 0.2s (`OnGatheringProgress` broadcast)
  - On complete: `HarvestStack(1, ...)` → `ResourceGathered(OutAmount, OutType)` → increment `CurrentGatheredStacks` → `ExecuteGathererModule(CurrentGatheringTarget)`
- Policy checks (in `Gather()`):
  - If carrying a different resource type than target → `RequestDeposit()`
  - If `CurrentGatheredStacks >= StacksStorageAmount` → `RequestDeposit()`
  - Else: request/keep slot and gather or move to slot
- Edge cases:
  - Resets `CurrentGatheredStacks` when module amount is 0 (after deposit)
  - Releases slot when resource is depleted

Events used:
- `UGathererModule::OnGatheringProgress`
- `UGathererModule::ResourceGathered`

Dependencies:
- `UGatherableModule` (for `GatheringTime`, `HarvestStack`)
- `USlotModule` (for slot reservation)
- `UGathererModule` (movement, state, signaling)


