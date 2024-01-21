#  Current State of Things
- Blue square that is generated is a Nursery. A Nursery is able to produce
  Ants. Initally, the only ant that can be produced from a Nursery is a Worker
  ant.

- In the future, I would like to make it so that you can make a deal with the
  Queen Ant to make different kinds of Ants from the nurseries.

- A key mechanic I think will be to allow certain areas of the map to only be
  accessable by certain ants. For example, there will be a lava section of the
  map which would only be accessable by fire ants. To be able to produce fire
  ants, you will need to bring a series of items to the Queen Ant.

- Each worker ant has two registers A and B, and a zero flag.
- You can use a simple assembly language to manipulate the hardware and move the ant.
- You can program instructions for new ants through the text editor by pressing "/".
- Once a program is intered into the text editor, you can close it and create a
  worker ant by occupying the nursery ant pressing 'a'.

# Current Controls

- MOVE [UP/LEFT/DOWN/RIGHT] - moves the ant one space in the respective direction.
- LDI [A/B] <INTEGER> - sets the A/B register to a constant.
- CPY [A/B] [A/B] - copys the value in one register to the other.
- ADD [A/B] [A/B] - adds the value of the second register to the first one.
- SUB [A/B] [A/B] - subtracts the value of second register from the first one.
- INC [A/B] - increments the register
- DEC [A/B] - decrements the register
- JMP [LABEL] - go to the label
- JNZ [LABEL] - got the label if the zero register is not zero.
- [LABEL]: - create a label

# Example Text EDITOR commands
```
TOP:
LDI A 5
LOOP:
MOVE DOWN
DEC A
JNZ LOOP
JMP TOP
```
