
# Command List
- ADD, COPY, DEC, INC, JNZ, JMP, LOAD, MOVE, NOP, SUB
- Total: 10

# Instruction Encoding (8 bit words)
- Supports 32 operations: 5 bits
- Additional 3 bits

# NOP Instruction (8 bits)
- Instruction: 0b00000
- Additional:  0b000

# Movement Instruction (8 bits)
- Instruction: 0b00001
- Up:          0b000
- Left:        0b001
- Down:        0b010
- Right:       0b011

# Load Constant Instruction (40 bits)
- Instruction: 0b00010
- Register A:  0b000
- Register B:  0b001
- Constant: next 32 bits (4 instructions) (little endian - least significant first)

# Two Register Instruction (8 bits)
- CPY (0b00011) / ADD (0b00100) / SUB (0b00101)
- Register A -> A: 0b000
- Register A -> B: 0b001
- Register B -> A: 0b010
- Register B -> B: 0b011

# Single Register Instruction (8 bits)
- INC (0b00110) / DEC (0b00111)
- Register A: 0b000
- Register B: 0b001

# Jump Instruction (24 bits)
- JMP (0b01000) / JNZ (0b01001)
- Additional: 0b000
- Address: next 16 bits (2 instructions)
