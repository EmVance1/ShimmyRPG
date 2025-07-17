# Shimmy Engine Schedule (+ indicates low prio, i.e. post alpha 1.0)

### Technical

- scripting and security
    - [x] event handler verification
    - [x] OnCreate / OnExit events
    - [x] merge lua states (env tables)
    - [x] minimal sandboxing to prevent io, os, debug table access
    - [ ] protect builtin tables (shallow copies)
    - [ ] preload scripts instead of lazy load

- debugging
    - [x] rapidjson error handlers
    - [x] lua pcalls / logging
    - [ ] total fallthrough in release mode (might need to rethink this one)
    - [ ] +script parser error reporting (maybe make rust lib)
    - [ ] +LSP for .shmy files

- navigation
    - [x] graph A*
    - [x] string pulling / line of sight
    - [x] discouraged regions
    - [ ] movement pentalty regions
    - [ ] fix weird behaviour inside discouraged regions

- sound and graphics
    - [x] test animation API
    - [x] minimal sound player class
    - [ ] minimal music player class
    - [ ] determine SFX management solution (dialogue being priority)
    - [ ] post processing config
    - [ ] post processing scripting
    - [ ] particle system config
    - [ ] particle system scripting
    - [ ] +automatic spritesheet generation from asset collections

- cult game issues
    - [x] gui style in region def
    - [x] fix broken "move to" action
    - [x] fix conditional dialogue bug (rng?)
    - [x] fix npc despawning after movement
    - [x] dialogue exit can trigger scripts (=> exit_into{ script })
    - [x] lockable/unlockable doors (hacky)
    - [x] better camera control (mostly zoom actions, on triggers) (super hacky)
    - [x] 'offstage' entities
    - [x] improve json error handling
    - [x] multithreaded background loading
    - [x] pooling of bg loader threads
    - [ ] grow tooltips to fit content
    - [ ] ANY kind of save files
    - [ ] better initial configuration

- miscellaneous
    - [x] ghost entities (uninteractible)
    - [x] shmy documentation
    - [ ] improve scripted zoom control
    - [ ] refine triggers (more shapes, concise actions)
    - [ ] improve handling of prefabs
    - [ ] appdata folder abstraction
    - [ ] json schema overhaul
    - [ ] json documentation
    - [ ] some way of reporting missing traits
    - [ ] redo door locking less hacky
    - [ ] full code style audit (namespacing, fixed conventions, API wrapping)


### Gameplay

- [ ] combat system design
    - [x] core mechanics
    - [ ] species bonuses
    - [ ] class features
    - [ ] marionette ability

- [ ] combat system implementation
    - [x] turn rotation
    - [x] factions
    - [x] per turn movement
    - [ ] entity stats
    - [ ] action gui
    - [ ] temp stat calculation (requires basic inventory)
    - [ ] targeting
    - [ ] cross-entity interaction
    - [ ] stand-in visuals


- [ ] character creator design
    - [ ] class features (see combat)
    - [ ] species features (see combat)
    - [ ] draw GUI
    - [ ] specify serialization

- [ ] character creator implementation


### Back Burner

- [ ] inventory system
- [ ] primitive dynamic lighting
- [ ] +swap out graphics backend
- [ ] +level editor
- [ ] +fine grained memory management
- [ ] +reactive flags (callbacks)
- [ ] +shmy_script as a library


### Final Boss Shit

- [ ] +actual usable build process for collaboration
- [ ] +leading into emscripten support
- [ ] +reactive gui scripting
- [ ] +campaign package registry
- [ ] +multiplayer???????


### Completed

- [x] fix visual glitching

- [x] navmesh generation (will probably not have usable real time perf. Edit: actually 0.3ms isnt bad)
    - [x] marching squares / floodfill
    - [x] contour merging
    - [x] douglas-peucker simplification
    - [x] hole punching
    - [x] ear clip triangulation
    - [x] delauney triangulation (used a library)
    - [x] serialization
    - [x] sexy editor babyyyyyy

