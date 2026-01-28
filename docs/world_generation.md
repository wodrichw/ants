# World Generation and World Hierarchy

This document summarizes how the world is generated and how the world is maintained in memory and on disk.

## High-level flow

1. **MapWorld creation**
   - A `MapWorld` is created with a `MapWindow` (visible border), `Regions`, and a list of `Level`s (one per depth).
   - Each `Level` owns a `Map` instance that holds `Chunk`s and `Tile`s.

2. **Region bootstrap**
   - The `Regions` container starts with a single origin region covering `(0,0)` through `globals::WORLD_LENGTH` in both axes.
   - That region is given seeds and immediately runs blueprint planning.

3. **Blueprint planning inside a region**
   - A `Region` subdivides its area into a grid of chunk coordinates.
   - The planner places `Zone`s (e.g., `Starting_Colony`, `Peaceful_Cavern`) into a 3D chunk grid (x, y, depth).
   - Each placed `Zone` becomes one or more `Section_Plan`s (one per depth slice of the zone), each with a rectangular border and a `build_section` callback.

4. **Section generation at runtime**
   - The map updates/loads chunks for the visible window.
   - For each unbuilt chunk, `MapManager::generate_sections()` calls `Regions::build_section()` with the chunk position and target `Level`.
   - `Regions::build_section()` locates or creates the correct `Region`, finds the matching `Section_Plan`, and invokes it to carve rooms/corridors into the `Map`.

## World hierarchy (conceptual)

The world is maintained in layers that separate **logical planning** from **runtime storage**:

1. **MapWorld**
   - Owns: `Levels`, `Regions`, and global state like the current depth.

2. **Level (depth)**
   - One per vertical layer (`z`).
   - Owns: `Map`, worker/building lists, and optional start data.

3. **Region (world partition)**
   - A square partition of size `globals::WORLD_LENGTH`.
   - Owns: random seeds and `section_plans` for all depths.

4. **Zone (planning only)**
   - A template shape for an area (e.g., a starting colony).
   - Used only during region blueprint planning to assign which areas become sections.

5. **Section_Plan (generation unit)**
   - A rectangular area with a `build_section` callback.
   - When invoked, it produces `MapSectionData`-like output and digs rooms/corridors into the map.

6. **Chunk (storage unit)**
   - A fixed grid block of tiles, size `globals::CHUNK_LENGTH`.
   - Used for lazy loading, saving, and update parity.

7. **Tile (smallest unit)**
   - Holds visibility, wall/floor state, and references to entities/buildings.

In short, the generation pipeline is:

**Region → Zone placement → Section_Plan → Map (chunks) → Tile**

## How regions are picked and expanded

- Regions are keyed by a `Region_Key` derived from world coordinates.
- When a chunk is requested, `Regions::build_section()` checks if a region exists for that coordinate:
  - If not, it snaps the coordinate to the region grid, derives new seeds from the origin region, and constructs a new `Region`.
  - The new region immediately runs blueprint planning, so its `Section_Plan`s exist before building.

## How sections carve the map

- When a `Section_Plan` is built, its callback typically:
  - Selects room and corridor rectangles.
  - Calls `Map::dig()` to turn walls into floor tiles inside those rectangles.
  - Marks chunks touched by that section as loaded.

## Persistence

- `Map`, `Chunks`, and `Region` data are serialized through protobuf messages.
- Seeds are persisted, which makes region generation deterministic on reload.

## Key tradeoffs in this design

- **Determinism:** Region seeds enable repeatable generation.
- **Streaming:** Chunk-level loading allows large worlds without full pre-generation.
- **Separation of concerns:** Zone/section planning is isolated from map storage and rendering.
