# Shimmy Script

## Basic Syntax
Shimmy script is at its core a graph representation. Named nodes are declared, representing the lines of NPCs, and edges makeup the possible responses / outcomes of a given interation. This reads as a call and response structure, wherein the programmer simply has to define what a given NPC or the player might have to say under any given circumstances, and the possibilities these may lead to. For example, the character 'Shimmy' welcomes the player into his pub, to which the player can respond in ways varying in positivity:
```shmy
v00 = Shimmy: [ "Greetings traveller. Please have a seat." ] => {
    "My what a lovely place you have here." => v10{ Shimmy_Approval: Add(5) },
    "Why thank you." => v11,
    "Mmmph. Alright then." => v12,
    ?(Player_Origin == Twinvayne) "Shimmy my old friend." => v13,
}
```
The basic structure of a node declaration is: "{vertex name} = {entity ID}: \[ {lines},+ \] => {outcome}" where 'outcome' is most often a list of player responses and the next vertex they each lead to, as shown above. The name of a vertex can be any string of the characters a-z, a-Z, 0-9 or _. Double quote '"' strings however, may be any unicode (utf-8) string. For simplicity, I will be using numbered IDs in this tutorial. The entity ID may also be replaced by the 'Narrator' keyword.
You may notice that the first response not only progresses the dialogue to vertex 'v10', but also appears to modify the flag 'Shimmy_Approval'. Any player response may Add, Sub, or Set any number of flags through a comma separated list of key-modifier pairs.
The 4th response demonstrates how a response may only be conditionally available. The parentheses may contain arbitrarily complex boolean expressions of flag ids, integer constants and boolean constants. Additionally, any non-zero integer result is interpreted as true.

## Control Flow

### Entry Point(s)
A dialogue script requires one or more entry-point nodes. An entry point has no name and contains a condition or 'default' marker as shown below:
```shmy
entry(Shimmy_Approval > 10) = Shimmy: [ ...
entry(default)              = Shimmy: [ ...
```
Entry point conditions are evaluated in order, and only one is ever selected, meaning that if more than one entry point condition is met, the first one evaulated is the one that is chosen. Therefore, it usually makes sense to list your entry points in order of priority. An error is thrown if no entry point is eligible.

### Gotos
One tool provided for code reuse is the ability to bypass player responses and jump straight from the end of one node to the start of another. The following shmy code:
```shmy
v00 = Shimmy: [ "Greetings traveller." ] => v01
v01 = Shimmy: [ "Please have a seat." ] => { ... }
```
Is logically identical to
```shmy
v00 = Shimmy: [ "Greetings traveller.", "Please have a seat." ] => { ... }
```
This is handy when for example multiple nodes converge on a single outcome, or for handing speaking control from one NPC to another, without having to go via a player response:
```shmy
v00 = Shimmy: [ "Thank goodness you're here." ] => v01
v01 = Brian:  [ "We need your help." ] => { ... }
```

### Exit Point(s)
There are two ways of defining ends to dialogue interactions.
The `exit` keyword can be used after any => symbol as though it were a vertex in itself. In that regard, an `exit` after a player response can also be followed by a flag modifier list. Reaching an exit quits the dialogue.
The other method is to use the `exit_into{ "path/to/script.lua" }` syntax. This statement spawns the provided script and runs it immediately, but is otherwise the same as a regular exit. At the time of writing this, these statements cannot result from player responses, and can only be used in gotos.

## Special Cases
A couple of flags that can be used in the engine behave in special ways. The two most notable are the `once` and `rng` flags.
The `once` flag automagically evaluates to true if and only if the node entry point it enables has never been used before (as of writing this, `once` does not yet work for response choices).
The `rng` flag is more of a syntactical element: writing an expression `rng_{id}{n}` sets a *new* flag `{id}` to be a random number in the range [0..n). This number can subsequently be accessed through `rng_{id}`. Note, the trailing number means the expression sets the rng variable, but also returns the generated integer like the numberless variant. Typical usage would look like the following:
```shmy
v00 = Shimmy: [ "What did you roll?" ] => {
    ?(rng_roll3 == 0) "I rolled a 1." => vroll1,
    ?(rng_roll  == 1) "I rolled a 2." => vroll2,
    ?(rng_roll  == 2) "I rolled a 3." => vroll3,
}
```

