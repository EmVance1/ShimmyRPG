# Shimmy Script

## Basic Syntax
Shimmy script is at its core a graph representation. Named nodes are declared, representing the lines of NPCs, and edges makeup the possible responses / outcomes of a given interaction. This reads as a call and response structure, wherein the programmer simply has to define what a given NPC or the player might have to say under any given circumstances, and the possibilities these may lead to. For example, the character 'Shimmy' welcomes the player into his pub, to which the player can respond in ways varying in positivity:
```shmy
v00 = Shimmy: [ "Greetings traveller. Please have a seat." ] => {
    "My what a lovely place you have here." => v10{ Shimmy.Approval += 5 },
    "Why thank you." => v11,
    "Mmmph. Alright then." => v12,
    (Player.Origin == Twinvayne) "Shimmy my old friend." => v13,
}
```
The basic structure of a node declaration is: "{vertex name} = {entity ID}: \[ {lines},+ \] => {outcome}" where 'outcome' is most often a list of player responses and the next vertex they each lead to, as shown above. The name of a vertex can be any string of the characters a-z, a-Z, 0-9 or _. Double quote '"' strings however, may be any unicode (utf-8) string. For simplicity, I will be using numbered IDs in this tutorial. The entity ID may also be replaced by the 'Narrator' keyword.
You may notice that the first response not only progresses the dialogue to vertex 'v10', but also appears to modify the flag 'Shimmy_Approval'. Any player response may Add, Sub, or Set any number of flags through a comma separated list of key-modifier pairs.
The 4th response demonstrates how a response may only be conditionally available. The parentheses may contain arbitrarily complex boolean expressions of flag ids, integer constants and boolean constants. Additionally, any non-zero integer result is interpreted as true. Syntax is mostly as you would expect from other languages -

## Control Flow

### Entry Point(s)
A dialogue script requires one or more entry-point nodes. An entry point has no name and contains a condition or 'default' marker as shown below:
```shmy
entry(Shimmy.Approval >= 10) = Shimmy: [ ...
entry(default)               = Shimmy: [ ...
```
Entry points are evaluated top to bottom, and only one is ever selected, meaning that if more than one entry point condition is met, the first one evaulated is the one that is chosen. Therefore, you should list your entry points in order of priority, or ensure that conditions are mutually exclusive. It is an error if on start no entry point is eligible. `default` nodes are not mandatory if you can ensure this is never the case.

### Gotos
One tool provided for code reuse is the ability to bypass player responses and jump straight from the end of one NPC line to the start of another. The following shmy code:
```shmy
v00 = Shimmy: [ "Greetings traveller." ] => v01
v01 = Shimmy: [ "Please have a seat." ]  => { ... }
```
Is semantically identical to
```shmy
v00 = Shimmy: [ "Greetings traveller.", "Please have a seat." ] => { ... }
```
This is handy when for example multiple nodes converge on a single outcome, or for handing speaking control back and forth directly between different NPCs:
```shmy
v00 = Shimmy: [ "Thank goodness you're here." ] => v01
v01 = Brian:  [ "We need your help." ]          => v02
v02 = Shimmy: [ "The situation is dire." ]      => exit
```

### Exit Point(s)
There are two ways of defining ends to dialogue interactions.
The `exit` keyword denotes a special node that will end the dialogue when reached. Other than that it behaves just like any other node (allowed in gotos, allows modifier lists etc.).
The other method is to use the `exit_with{ "('{event}', {args})" }` syntax. In addition to ending the dialogue, this invokes the stated event with the stated args (see lua chapter for more information about events).

## Special Cases
A couple of flags that can be used in the engine behave in special ways. These are the temp, `once` and `rng` flags.
To define a temporary flag, prefix it with an underscore. Temporary flags are all deleted once an exit point is reached. These are best used in combination with the compound assignment operator `:` as shown below.
The `once` flag automagically evaluates to 1 (true) if and only if the node entry point it enables has never been used before (at time of writing, `once` does not yet work for response choices).
The `rng` flag is more of a function: writing the expression `rng{n}`, where `n` is any integer > 0, returns a random number in the range [0, n). This number is temporary. If you want to be able to access it more than once, you can use the compound assignment operator `:`, as in the following example:
```shmy
v00 = Shimmy: [ "What did you roll?" ] => {
    (_roll:rng3 == 0) "I rolled a 1." => vroll1,
    (_roll      == 1) "I rolled a 2." => vroll2,
    (_roll      == 2) "I rolled a 3." => vroll3,
}
```
As you can see, a temp flag `_roll` is created and set to the return value of `rng3`. Therefore in this configuration, there is only ever one possible response. It is up to the user to ensure naming collision is avoided when using this syntax. By convention, you should use temporaries or lowercase names for flags that you wont care about for very long, or won't be used outside of this script, and reserve CamelCase names for long lived flags.

