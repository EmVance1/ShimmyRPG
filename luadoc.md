# Lua Scripting API

## Execution Environment
In order to provide some degree of safety for users executing potentially untrusted scripts, the interface exposes a restricted subset of the lua standard library. The library tables that remain usable are `math`, `string`, `table`, `coroutine`, as well as the functions `print`, `pcall`, `pairs`, `ipairs`, `tonumber`, `tostring`, `type`, `next`, `select` and `unpack`.

## Overridable Functions
The Shimmy engine expects every script to override one or more of the following event handlers. In order for your scripts to behave predictably, all code that directly interacts with the scene must be inside these event handlers. Some of these handlers are available in both regular and coroutine variants (see section on [yields](#Yields)). If possible, the regular variant should be preferred, as it incurs lower overhead. It is *not* legal to define both versions of the same handler.

### OnCreate
```lua
function OnCreate() ... end
```
If defined, this function is called when the script is loaded into the scene (usually while the scene itself is loading).

### OnStart
```lua
function OnStart() ... end
```
If defined, this function is called when the script is activated by a trigger or a dialogue `exit_with{}` statement. If this takes place immediately when the script is loaded in, the function is nonetheless guaranteed to run *after* `OnCreate`.

### OnStartAsync
```lua
function OnStartAsync() ... end
```
Async (coroutine) variant of `OnStart`. Enables [`yield`](#Yields) functionality. If it yields, this handler *may* run concurrently with other handlers *within the same script*, such as an `OnUpdate` handler.

### OnUpdate
```lua
function OnUpdate(deltatime) ... end
```
If defined, this function is called every frame *after* the `OnStart` event is triggered (but in the case of `OnStartAsync`, potentially before the handler completes). The time delta between frames is passed to the function.

### OnUpdateAsync
```lua
function OnUpdateAsync(deltatime) ... end
```
Async (coroutine) variant of `OnUpdate`. Enables [`yield`](#Yields) functionality. If `OnUpdateAsync` yields, it will *not* be called in following frames until the yielded call is resumed and executed to completion.

### OnExit
```lua
function OnExit() ... end
```
If defined, this function is called immediately before either the script unloads itself, or the encompassing scene is unloaded. It is illegal for any code to run after this function returns. As such, this handler is guaranteed to run *after* the last call to `OnUpdate`. Any defined coroutine that is in a yielded state at the time of this event will be cut short to prevent invalid accesses.

## Engine API
The Shimmy engine exposes a number of functions for querying and manipulating the active scene through the `shmy` table. This table is in scope by default and doesn't need to be `require`d. These functions can be divided into two categories: synchronous functions and yields. Yields can *only* be called from event handlers labeled 'Async'. They hand control back to the engine for some specified amount of time or for the duration of some action, and are sometimes necessary to make the scene respond to your requests at the correct time.

### Aside - Flag Table
Flags can be read from and written to using the `shmy.flags` pseudotable. This table automagically forwards any reads and writes to the engine using metatable trickery, enabling the following syntax:
```lua
shmy.flags["MyFlag"] = 5
if shmy.flags["MyOtherFlag"] then
    -- some code
end
```
If this however feels *too* magical, the (identical) functions `shmy.flags.get(name)`, `shmy.flags.set(name, value)` and `shmy.flags.create(name, value)` are provided, `create` being the only way to spawn new flags at runtime. Any read/write to a nonexistent flag by other means will result in a crash.

### Aside - Vectors and Colors
Whenever a function is stated to take or return a Vector2, the table in question is defined as `{ x={x}, y={y} }` where x and y are *named* real number fields.
Whenever a function is stated to take or return a Color, the table in question is defined as `{ {r}, {g}, {b} }` where r, g and b are *indexed* integers between 0 and 255.

### Synchronous Functions
```lua
shmy.entity(script_id: string): Entity
```
Takes the script ID of an entity that exists in the current scene. Returns an Entity object (see section on [entities](#Entities)) which provides a number of functions for interacting with the entity.
```lua
shmy.set_mode(mode: integer)
```
Sets the current gamemode. Calling this function is a necessary prerequisite to any call to `yield_to_dialogue` or `yield_to_combat`. All gamemodes are enumerated in the table `shmy.mode`.
```lua
shmy.set_overlay(color: Color)
```
Sets a color which is blended (multiplied) with the entire screen (excluding GUI). This effect can be reversed using `shmy.set_overlay({ 255, 255, 255 })`.
```lua
shmy.goto_area(index: integer, spawn_pos: Vector2, suppress_triggers: boolean)
```
Sets the active area to the one provided.
```lua
shmy.exit()
```
Marks the script for termination as soon as control is returned to the engine. This does *not* in itself interrupt the execution of the current event handler (see `shmy.yield_exit()`), but it does guarantee that the calling handler is the last handler to be run or resumed, aside from of course `OnExit`.

##### Camera
```lua
shmy.camera.set_position(position: Vector2)
```
Sets the position of the scene camera.
```lua
shmy.camera.get_position(): Vector2
```
Returns the position of the scene camera.
```lua
shmy.camera.set_target(target: Vector2)
```
Sets the tracking target of the scene camera.
```lua
shmy.camera.set_zoom(zoom: number)
```
Sets the zoom factor of the scene camera. 1.0 is the default zoom level, 2.0 is zoomed in such that the camera covers half the usual width and height, etc.

##### Entities
The table returned by a call to `shmy.entity` contains the following functions, as well as a pointer to the C entity object itself. As such, each of these can benefit from lua method syntax `entity:method()` to implicitly pass the `self` argument.
```lua
Entity:set_position(self, position: Vector2)
```
Sets the position of an entity. This function silently fails if the position is not traversible.
```lua
Entity:get_position(self): Vector2
```
Retrieves the position of an entity.
```lua
Entity:set_path(self, target: Vector2)
```
Sets the pathfinding target of an entity. Calling this function causes an unlocked entity to immediately begin moving towards the target. This function silently fails if the target position is either not traversible or not reachable from the current position.
```lua
Entity:set_pathing_locked(self, locked: boolean)
```
Sets the `locked` state of an entity. An entity that is locked cannot pathfind, but can still be moved manually by a call to `set_position`.
Note: if an entity is locked while moving, its current path is dropped. If you wish for an entity to resume any previous path, use `set_path_paused`.
```lua
Entity:set_pathing_paused(self, paused: boolean)
```
Sets the `paused` state of an entity. An entity that is paused cannot pathfind, but can still be moved manually by a call to `set_position`. Unpausing an entity will cause it to resume the path it was previously on, if available.
```lua
Entity:set_offstage(self, offstage: boolean)
```
Sets the `offstage` state of an entity. An entity that is offstage is both invisible and uninteractible, thus in effect non-existent. This is preferred over on the fly spawning/despawning as entities are slow to load but cheap to store.
```lua
Entity:set_animation(self, index: integer)
```
Sets the row of an entities spritesheet to be used when playing animations.
Note: in the case of dynamic entities, the first 8 rows are reserved for directional movement.

### Yields
```lua
shmy.yield()
```
Yields control back to the engine for a single frame before resuming the function. May be necessary if the engine needs to propagate some internal state from your commands before you can continue. Usually however, this is not needed.

```lua
shmy.yield_seconds(seconds: number)
```
Yields control back to the engine for some number of seconds (fractions allowed). This is an easy way to specify delays between actions.
Note: wakeup is only checked once per frame, therefore all delays are rounded up to the next multiple of 1/60. This however is only noticeable when passing in small delays. For precise timing logic, prefer manually tracking `deltatime`.

```lua
shmy.yield_to_dialogue(filepath: string)
```
Loads a dialogue script from the given filepath, and yields control to the dialogue player. The calling script is resumed immediately once the dialogue reaches an `exit` statement.
Note: when spawning dialogue from a script, it is technically legal for said dialogue to exit into a newly loaded script using `exit_into{}`, as the engine supports concurrency in that regard. This however is not advised, as handling terminated dialogue using the script that spawned it should be preferred if possible. Use `exit_into{}` for dialogues spawned in isolation (as in not from a script).

```lua
shmy.yield_to_combat(ally_tags: table, enemy_tags: table)
```
Analogous to `shmy.yield_to_dialogue` - constructs a combat encounter from the provided tag arrays and yields control to the engine until the encounter reaches any valid conclusion state. This conclusion state is saved to the reserved "CombatConclusion" flag which can be queried like any other, and compared against the constants stored in `shmy.combat_conclusions` to handle consequences.
```lua
shmy.yield_exit()
```
Terminates execution of the current script immediately. Additionally it is guaranteed that the caller is the last handler to be run or resumed, aside from of course `OnExit`.

