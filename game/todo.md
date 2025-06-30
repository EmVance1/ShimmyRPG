# Shimmy Engine Schedule

### In Progress

- [ ] DESIGN combat system
    - [x] core mechanics
    - [ ] species bonuses
    - [ ] class features
    - [ ] marionette ability

- [ ] IMPLEMENT combat system
    - [ ] entity stats
    - [x] turn rotation
    - [x] factions
    - [x] per turn movement
    - [ ] action gui
    - [ ] temp stat calculation (requires basic inventory)
    - [ ] targeting
    - [ ] cross-entity interaction
    - [ ] stand-in visuals


- [ ] IMPROVE error handling and logging
    - [ ] script parser error reporting
    - [x] rapidjson error handlers
    - [x] lua pcalls / logging
    - [x] total fallthrough in release mode


- [ ] DESIGN character creator
    - [ ] class features (see combat)
    - [ ] species features (see combat)
    - [ ] draw GUI
    - [ ] specify serialization

- [ ] IMPLEMENT character creator


- [ ] IMPLEMENT navmesh generation (will probably not have usable real time perf) (actulally 0.3ms isnt bad)
    - [x] marching squares / floodfill
    - [x] contour merging
    - [x] douglas-peucker simplification
    - [x] hole punching
    - [x] ear clip triangulation
    - [x] delauney triangulation (used a library)
    - [x] serialization
    - [x] sexy editor babyyyyyy

- [ ] IMPLEMENT navmesh navigation
    - [x] graph A*
    - [x] string pulling / line of sight
    - [x] discouraged regions
    - [ ] movement pentalty regions
    - [ ] fix weird behaviour inside discouraged regions


- [ ] open issues from proof of concept cult game:
    - [x] gui style in region def
    - [x] fix broken "move to" action
    - [x] fix conditional dialogue bug (rng?) we say maybe
    - [x] fix npc despawning after movement
    - [x] dialogue exit can trigger scripts (=> exit_into{ script })
    - [x] lockable/unlockable doors (hacky)
    - [x] better camera control (mostly zoom actions, on triggers) (super hacky)
    - [x] 'offstage' entities
    - [x] improve json error handling
    - [ ] better handling of textures
    - [ ] grow tooltips to fit content
    - [ ] better initial configuration
    - [ ] ANY kind of save files


### Back Burner

- [ ] IMPLEMENT inventory

- [ ] IMPLEMENT primitive dynamic lighting

- [ ] IMPROVE level editor

- [ ] IMPROVE memory management / scripting


### Completed

- [x] FIX visual glitching

- [x] IMPLEMENT frustum loading

