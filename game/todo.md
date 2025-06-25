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
    - [ ] factions
    - [ ] temp stat calculation (requires basic inventory)
    - [ ] targeting
    - [ ] cross-entity interaction
    - [ ] stand-in visuals


- [ ] DESIGN character creator
    - [ ] class features (see combat)
    - [ ] species features (see combat)
    - [ ] draw GUI
    - [ ] specify serialization

- [ ] IMPLEMENT character creator


- [ ] IMPLEMENT navmesh generation (will probably not have usable real time perf) (actulally 0.3ms isnt bad
    - [x] marching squares / floodfill
    - [x] contour merging
    - [x] douglas-peucker simplification
    - [x] hole punching
    - [x] ear clip triangulation
    - [x] delauney triangulation (used a library)
    - [x] serialization

- [ ] IMPLEMENT navmesh navigation
    - [x] graph A*
    - [ ] string pulling / line of sight


### Back Burner

- [ ] IMPROVE error handling and logging (in debug mode)

- [ ] IMPLEMENT inventory

- [ ] IMPLEMENT primitive dynamic lighting

- [ ] IMPROVE level editor

- [ ] IMPROVE memory management / scripting


### Completed

- [x] FIX visual glitching

- [x] IMPLEMENT frustum loading

